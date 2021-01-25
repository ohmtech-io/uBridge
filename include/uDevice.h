#pragma once

#include <chrono>
#include <nlohmann/json.hpp>

#include "threadsafeQueue.h"
#include "uSerial.h"


namespace ubridge {
using json = nlohmann::json;

using PortName = std::string;
using PortObject = LibSerial::SerialPort;
using PortList = std::vector<PortName>;	

class Bridge;

struct uThingQueries_t {
	const char* info = "{\"info\": true}\n";
	const char* status = "{\"status\": true}\n";
	const json j_info = json::parse(info);
	const json j_status = json::parse(status);
};

class Uthing {
public:	
	Uthing(const PortName& portName, PortObject portObj);

	//getters:
	std::string portName();
	std::string devName(); 
	std::string channelID();
	std::string fwVersion();
	std::string serialNumber();
	// upTime() {return lastUpTime + ******}; https://github.com/AnthonyCalandra/modern-cpp-features#stdchrono
	auto messagesReceived();
	auto messagesSent();
	json info(); /* This is static for a device */
	json status();

	// void setChannelID(const std::string& channelID);
	void assignChannelID();

	void relayThread(Bridge& bridge);
	// void relayThread(TQueue<json>& inboundQueue, TQueue<json>& outboundQueue);
	
private:
	json query(const char* query);

private:
	//Port:
	PortName _portName;
	PortObject _port;

	//Device:
	std::string _devName;	//exampole: "uThing::VOC rev.A", multiple devices with the same name possible
	std::string _fwVersion;
	std::string _serialNumber;
	//used for Pub/Sub
	std::string _channelID; //unique

	//some stats:
	std::chrono::milliseconds _lastUpTime;
	int _messagesReceived = 0;
	int _messagesSent = 0;		
}; //class 
} //namespace ubridge