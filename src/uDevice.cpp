#define LOGURU_WITH_STREAMS 1
#include <loguru/loguru.hpp>
#include <thread>

#include "uSerial.h"
#include "ubridge.h"
#include "uDevice.h"

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

auto Uthing::portName() {return _portName;}
auto Uthing::devName() {return _devName;}
auto Uthing::channelID() {return _channelID;}
auto Uthing::fwVersion() {return _fwVersion;}
auto Uthing::serialNumber() {return _serialNumber;}
// upTime() {return lastUpTime + ******}; https://github.com/AnthonyCalandra/modern-cpp-features#stdchrono
auto Uthing::messagesReceived() {return _messagesReceived;}
auto Uthing::messagesSent() {return _messagesSent;}

/* This is static for a device */
json Uthing::info() {
	json info;
	info["info"]["device"] = _devName;
	info["info"]["serial"] = _serialNumber;
	info["info"]["firmware"] = _fwVersion;
	return info;
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
				inMessage.clear();
				_port.ReadLine(inMessage, '\n', 10);
				LOG_S(9) << inMessage;
				
				json jmesg = json::parse(inMessage);
				
				bridge.inboundQ.push(std::move(jmesg));			
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

		//100 Hz poll rate (app taking ~1% of CPU on Ubuntu virtual machine)
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		//TODO: what if we slow down this and allow for potentially faster sensors
		// to buffer (need to figure out the size of the buffer), and we read and forward in batches
	}
}

} //namespace ubridge
