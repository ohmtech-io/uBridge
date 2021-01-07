#pragma once

#include <libserial/SerialPort.h>


namespace ubridge {

using PortName = std::string;
using PortObject = LibSerial::SerialPort;
using PortList = std::vector<PortName>;

const struct uThingQueries_t {
	const char* info = "{\"info\": true}\n";
	const char* status = "{\"status\": true}\n";
	const json j_info = json::parse(info);
	const json j_status = json::parse(status);
} uThingQueries;

void findPorts(std::string &devNameBase, PortList &portList) {

	//TODO: mutex here?
	portList.clear();

	std::string path = "/dev";

	const std::regex expr(devNameBase + "\\d");

	/* list files on /dev and filter with the provided name base*/
	for (const auto& file : std::filesystem::directory_iterator(path)) {

		if (std::regex_match(file.path().string(), expr)) {
		 	LOG_S(6) << "Port found: " << file.path();
			portList.push_back(file.path());	 
		}
	}
}

bool isUthing(const PortName &fileDescriptor, PortObject &port) {
	try {
		port.Open(fileDescriptor);

		if (port.IsOpen()) {
			LOG_S(9) << "Port " << fileDescriptor << " is Open";
			/*let's give some time to the uTHing device to dump the buffer*/
			std::this_thread::sleep_for(100ms);
		} else return false;
	} catch (const std::exception& ex) {
		LOG_S(WARNING) << fileDescriptor << ": " << ex.what();
		return false;
	}

	/* The uThing:: devices are always sending data, so a few things can happen:
		1- we try to query the device exactly at the same time when the device is sending a data point
			so we will get a datapoint instead of the request's reply.
		2- the USB stack on the device has a TX buffer. If the device is connected and transmitting before 
			the port is open, a few of data messages will be already waiting to be read.
		To overcome these situations, why we need to try the request a few times.
	*/
	for (int i = 0; i < 5; ++i)
	{
		port.FlushIOBuffers();	

		LOG_S(9) << "Request {info}";		
		// port.Write("{\"info\": true}\n");
		port.Write(uThingQueries.info);
		
		try {	
			std::string response;
			port.ReadLine(response, '\n', 1500);
			LOG_S(9) << "Read " << response;

			json jmesg = json::parse(response);
			LOG_S(9) << "JSON resp " << jmesg;	

			if (jmesg.contains("info")) {
				LOG_S(6) << jmesg["info"]["device"];
				return true;
			}			
		} catch (const std::exception& ex) {
			LOG_S(WARNING) << fileDescriptor << ": " << ex.what();
		}

		std::this_thread::sleep_for(10ms);
	}
	//couldn't get the info (not a uThing device probably?)	
	return false;
}

class Uthing {
public:	
	Uthing(const PortName& portName, PortObject portObj) {
		_portName = portName;
		_port = std::move(portObj);

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
	// upTime() {return lastUpTime + ******};
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
			// json jerr["error"] = "Failed to get " + std::string(query); 
			json jerr;
			// jerr["error"] = "Failed to query the device";
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
	std::string _channelID; //unique
	std::string _fwVersion;
	std::string _serialNumber;

	//some stats:
	std::chrono::milliseconds _lastUpTime;
	size_t _messagesReceived = 0;
	size_t _messagesSent = 0;		
}; //class 
} //namespace ubridge