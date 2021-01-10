#pragma once

#include "uSerial.h"

namespace ubridge {

const struct uThingQueries_t {
	const char* info = "{\"info\": true}\n";
	const char* status = "{\"status\": true}\n";
	const json j_info = json::parse(info);
	const json j_status = json::parse(status);
} uThingQueries;


class Uthing {
public:	
	Uthing(const PortName& portName, PortObject portObj): 
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
	// ~Uthing(){};

	auto portName() {return _portName;}
	auto devName() {return _devName;}
	auto channelID() {return _channelID;}
	auto fwVersion() {return _fwVersion;}
	auto serialNumber() {return _serialNumber;}
	// upTime() {return lastUpTime + ******}; https://github.com/AnthonyCalandra/modern-cpp-features#stdchrono
	auto messagesReceived() {return _messagesReceived;}
	auto messagesSent() {return _messagesSent;}
	

	/* This is static for a device */
	json info() {
		json info;
		info["info"]["device"] = _devName;
		info["info"]["serial"] = _serialNumber;
		info["info"]["firmware"] = _fwVersion;
		return info;
	}

	json status() {return query(uThingQueries.status);}
	
private:
	json query(const char* query) {
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

private:
	//Port:
	PortName _portName;
	PortObject _port;

	//Device:
	std::string _devName;	//multiple devices with the same name possible
	std::string _fwVersion;
	std::string _serialNumber;
	//used for Pub/Sub
	std::string _channelID; //unique

	//some stats:
	std::chrono::milliseconds _lastUpTime;
	size_t _messagesReceived = 0;
	size_t _messagesSent = 0;		
}; //class 
} //namespace ubridge