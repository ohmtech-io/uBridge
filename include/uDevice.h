#pragma once

#include <chrono>
#include <nlohmann/json.hpp>

#include "uSerial.h"

namespace ubridge {
using json = nlohmann::json;

using PortName = std::string;
using PortObject = LibSerial::SerialPort;
using PortList = std::vector<PortName>;	


struct uThingQueries_t {
	const char* info = "{\"info\": true}\n";
	const char* status = "{\"status\": true}\n";
	const json j_info = json::parse(info);
	const json j_status = json::parse(status);
};

class Uthing {
public:	
	Uthing(const PortName& portName, PortObject portObj);

	auto portName();
	auto devName(); 
	auto channelID();
	auto fwVersion();
	auto serialNumber();
	// upTime() {return lastUpTime + ******}; https://github.com/AnthonyCalandra/modern-cpp-features#stdchrono
	auto messagesReceived();
	auto messagesSent();
	

	/* This is static for a device */
	json info();

	json status();
	
private:
	json query(const char* query);

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