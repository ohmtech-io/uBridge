#define LOGURU_WITH_STREAMS 1
#include <loguru/loguru.hpp>
#include <nlohmann/json.hpp>
#include <thread>
#include "uSerial.h"
#include "ubridge.h"
#include "uDevice.h"

#include "testTimer.h"

namespace ubridge {

const uThingQueries_t uThingQueries;

Uthing::Uthing(const PortName& portName, PortObject portObj): 
					_portName(portName), 
					_port(std::move(portObj)) 
{
	json resp = query(uThingQueries.info);
	if (resp.contains("error")) {
		 throw std::runtime_error("failed to gather device information"); 
	} 
	_devName = resp["info"]["device"];
	_fwVersion = resp["info"]["firmware"];
	_serialNumber = resp["info"]["serial"];
}

std::string Uthing::portName() {return _portName;}
std::string Uthing::devName() {return _devName;}
std::string Uthing::channelID() {return _channelID;}
std::string Uthing::fwVersion() {return _fwVersion;}
std::string Uthing::serialNumber() {return _serialNumber;}
// upTime() {return lastUpTime + ******}; https://github.com/AnthonyCalandra/modern-cpp-features#stdchrono
auto Uthing::messagesReceived() {return _messagesReceived;}
auto Uthing::messagesSent() {return _messagesSent;}

/* This is static for a device (obtained during initialization) */
json Uthing::info() {
	json info;
	info["info"]["device"] = _devName;
	info["info"]["serial"] = _serialNumber;
	info["info"]["firmware"] = _fwVersion;
	return info;
}

// void Uthing::setChannelID(const std::string& channelID) {
// 	_channelID = channelID;
// }

void Uthing::assignChannelID() {
	//-----"device": "uThing::VOC rev.A"----
	//remove " rev.X"
	std::string baseName = _devName.substr(0, _devName.size() - 6);
	_channelID = _devName + '_' + _serialNumber.substr(_serialNumber.length()-4, _serialNumber.length());
	
	LOG_S(6) << "dev name: " << _devName <<", ChannelID: " << _channelID;
}


json Uthing::status() {return query(uThingQueries.status);}
	
json Uthing::query(const char* query) {
	try {	
		_port.FlushIOBuffers();	

		LOG_S(6) << "Requesting "<< query;		
		_port.Write(query);
		
		std::string response;
		_port.ReadLine(response, '\n', 1500);
		LOG_S(9) << "Read " << response;

		json jmesg = json::parse(response);
		LOG_S(6) << "JSON resp " << jmesg;	
		return jmesg;
	} catch (const std::exception& ex) {
		LOG_S(WARNING) << _portName << _devName << ": " << ex.what();
		json jerr;
		jerr["error"] = "Failed to get " + std::string(query); 
		return jerr;
	}
}

void Uthing::relayThread(Bridge& bridge) {
	
	LOG_S(INFO) << "Device thread created..." << info();
	std::string inMessage;

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
				// For some reason, IsOpen() is always returning true, so in order to detect
			 	// a device being detached we try an I/O operation, which 
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
			_port.Close();
			bridge.devices.erase(_portName);

			return;	
		}
		catch (const json::exception& ex) {
			LOG_S(WARNING) << _portName << ": " << ex.what();
		}

		//100 Hz poll rate (app taking ~3-4% of CPU on Ubuntu virtual machine)
		//10 Hz with batched messages below 1%
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

} //namespace ubridge
