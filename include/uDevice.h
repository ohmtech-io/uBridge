#pragma once

#include <libserial/SerialPort.h>


namespace ubridge {

using PortName = std::string;
using PortObject = LibSerial::SerialPort;
using PortList = std::vector<PortName>;

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


// PortObject tempPort;
// if (isUthing(portName, tempPort)) {
// 	create new Uthing
// 	move tempPort to it
// 	push it to the uThings vector
// }

bool isUthing(const PortName &fileDescriptor, PortObject &port) {
	try {
		port.Open(fileDescriptor);

		if (port.IsOpen()) {
			LOG_S(9) << "Port " << fileDescriptor << " is Open";
		} else return false;
	} catch (const std::exception& ex) {
		LOG_S(WARNING) << fileDescriptor << ": " << ex.what();
		return false;
	}
	std::string response;
	json jmesg;

	/* The uThing:: devices are always sending data, so a few things can happen:
		1- we try to query the device exactly at the same time when the device is sending a data point
			so we will get a datapoint instead of the request's reply.
		2- the USB stack on the device has a TX buffer. If the device is connected and transmitting before 
			the port is open, a few of data messages will be already waiting to be read.
		To overcome these situations, why we need to try the request a few times.
	*/
	for (int i = 0; i < 10; ++i)
	{
		port.FlushIOBuffers();	

		LOG_S(9) << "Request info";		
		port.Write("{\"info\": true}\n");
		// port.DrainWriteBuffer();
	
		try {		
			port.ReadLine(response, '\n', 1500);
			LOG_S(9) << "Read " << response;

			jmesg = json::parse(response);
			LOG_S(9) << "JSON resp " << jmesg;	

			if (jmesg.contains("info")) {
				return true;
			}			
		} catch (const std::exception& ex) {
			LOG_S(WARNING) << fileDescriptor << ": " << ex.what();
		}

		std::this_thread::sleep_for(50ms);
	}
	//couldn't get the info (not a uThing device probably?)	
	return false;
}


// typedef std::string portName_t;	
// typedef std::vector<portName_t> portList_t;

class Uthing {
public:	
	// Uthing() {
	// }
	// ~Uthing(){};

	// int open(const portName_t& name) {
	// 	try {
	// 		/* using move constructor: https://github.com/crayzeewulf/libserial/pull/165 */
	// 		ports.push_back(LibSerial::SerialPort(name));

	// 		if (ports.back().IsOpen()) {
	// 			LOG_S(9) << "Port " << name << " is Open";
	// 			return 0;
	// 		} else return -1;

	// 	} catch (const std::exception& ex) {
	// 		LOG_S(WARNING) << name << ": " << ex.what();
	// 		return -1;
	// 	}
	// 	return -1;
	// }



	// int close(portName_t& name) {
	// 	return 0;
	// }

public:
	using PortName = std::string;
	using PortObject = LibSerial::SerialPort;
	using PortList = std::vector<PortName>;
		//Port:
	PortName portName;
	PortObject port;

	//Device:
	std::string devName;	//multiple devices with the same name possible
	std::string channelID; //unique
	std::string fwVersion;
	std::string serialNumber;

	//some stats:
	std::chrono::milliseconds upTime;
	size_t messagesReceived = 0;
	size_t messagesSent = 0;		

// private:
	// portList_t *portsNameList;
	// std::vector<LibSerial::SerialPort> ports;

}; //class 
} //namespace ubridge