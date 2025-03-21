/***************************************************************************
*** MIT License ***
*
*** Copyright (c) 2020-2021 Daniel Mancuso - OhmTech.io **
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.     
****************************************************************************/

#define LOGURU_WITH_STREAMS 1
#include <loguru/loguru.hpp>
#include <nlohmann/json.hpp>
#include <thread>
#include "uSerial.h"
#include "ubridge.h"
#include "uDevice.h"

// #include "testTimer.h"

namespace ubridge {

const uThingQueries_t uThingQueries;

Uthing::Uthing(const PortName& portName): 
					_portName(portName)
{
	_port.Open(_portName);
	json resp = query(uThingQueries.info);
	if (resp.contains("error")) {
		 throw std::runtime_error("failed to gather device information"); 
	} 
	_devName = resp["info"]["device"];
	_fwVersion = resp["info"]["firmware"];
	_serialNumber = resp["info"]["serial"];
}

// Add move constructor
Uthing::Uthing(Uthing&& other) noexcept:
    _portName(std::move(other._portName)),
    //_port(std::move(other._port)),
    _devName(std::move(other._devName)),
    _fwVersion(std::move(other._fwVersion)),
    _serialNumber(std::move(other._serialNumber)),
    _channelID(std::move(other._channelID)),
    _lastUpTime(other._lastUpTime),
    _messagesReceived(other._messagesReceived),
    _messagesSent(other._messagesSent)
{
    // Reset counters and values in the source object
    other._lastUpTime = 0;
    other._messagesReceived = 0;
    other._messagesSent = 0;
}


//getters:
std::string Uthing::portName() {return _portName;}
std::string Uthing::devName() {return _devName;}
std::string Uthing::channelID() {return _channelID;}
std::string Uthing::fwVersion() {return _fwVersion;}
std::string Uthing::serialNumber() {return _serialNumber;}
int Uthing::upTime() {return _lastUpTime;}// https://github.com/AnthonyCalandra/modern-cpp-features#stdchrono
int Uthing::messagesReceived() {return _messagesReceived;}
int Uthing::messagesSent() {return _messagesSent;}

/* This is static for a device (obtained during initialization) */
json Uthing::info() {
	json info;
	info["info"]["device"] = _devName;
	info["info"]["serial"] = _serialNumber;
	info["info"]["firmware"] = _fwVersion;
	return info;
}

void Uthing::assignChannelID() {
	//-----"device": "uThing::VOC rev.A"----
	//remove " rev.X"
	std::string baseName = _devName.substr(0, _devName.size() - 6);
	_channelID = baseName + '_' + _serialNumber.substr(_serialNumber.length()-4, _serialNumber.length());
	
	LOG_S(6) << "dev name: " << _devName <<", ChannelID: " << _channelID;
}

json Uthing::status() {
	return query(uThingQueries.status); 
}
	
json Uthing::jquery(const json& jquery) {
	LOG_S(9) << "jquery: " << jquery;
	return query(jquery.dump().c_str());
}	

json Uthing::query(const char* raw_query) {
	try {	
		_port.FlushIOBuffers();	

		LOG_S(6) << "Requesting "<< raw_query;		
		_port.Write(raw_query);
		++_messagesSent;

		std::string response;
		_port.ReadLine(response, '\n', 1500);
		LOG_S(9) << "Read " << response;
		++_messagesReceived;

		json jmesg = json::parse(response);
		LOG_S(6) << "JSON resp " << jmesg;	

		if (jmesg["status"].contains("upTime")) {
			int value = jmesg["status"]["upTime"];
			_lastUpTime = value;
		}

		return jmesg;
	} catch (const std::exception& ex) {
		LOG_S(WARNING) << _portName << _devName << ": " << ex.what();
		json jerr;
		jerr["error"] = "Failed to get " + std::string(raw_query); 
		return jerr;
	}
}

void Uthing::relayThread(Bridge& bridge) {
	
	loguru::set_thread_name(_channelID.c_str());

	LOG_S(INFO) << "Device thread created..." << info();
	std::string inMessage;

	_port.Open(_portName);

	while(true) {
		try {
			if (_port.IsDataAvailable()) {
				//retrieve the messages in batch (queued on the port FIFO)
				// in order to reduce the CPU load due to the threading allocation overhead
				for (auto i = 0; i < 100; ++i)
				{
					if (_port.IsDataAvailable()) {
						_port.ReadLine(inMessage, '\n', 10);
						
						LOG_S(9) << inMessage;
						json jmesg = json::parse(inMessage);

						jmesg["channelID"] = _channelID;				
						++_messagesReceived;
						
						bridge.inboundQ.push(std::move(jmesg));	
					} else break;
				}
			} else {
				// For some reason, IsOpen() is always returning true (even if the device was removed), 
			 	// so in order to detect a device being detached we try an I/O operation, which 
			 	// will throw an exception (Input/output error) if the port is not available anymore.	 
				_port.FlushOutputBuffer();
			}
		}  
		catch (const LibSerial::ReadTimeout& ex) {
			LOG_S(WARNING) << _portName << "->" << _devName << ": " << ex.what();
		}
		catch (const std::runtime_error& ex) {
			//Likely thrown due to inexistent port (when we remove the device)
			LOG_S(WARNING) << _portName << "->" << _devName << ": " << ex.what();
			//So we close the port, delete the device and finish this thread
			LOG_S(INFO) << "Removing "<< _devName << " on " << _portName;
			
			const std::lock_guard<std::mutex> lck(bridge.mutex_devices);	
			//_port.Close();
			// bridge.devices.erase(_portName);

			return;	
		}
		catch (const json::exception& ex) {
			LOG_S(WARNING) << _portName << ": " << ex.what();
		}

		//100 Hz poll rate (app taking ~3-4% of CPU on Ubuntu virtual machine)
		//10 Hz with batched messages, well below 1%
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

} //namespace ubridge
