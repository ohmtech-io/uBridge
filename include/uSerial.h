#pragma once

#include <libserial/SerialPort.h>

namespace ubridge {

using PortName = std::string;
using PortObject = LibSerial::SerialPort;
using PortList = std::vector<PortName>;	

class Uthing;

void findPorts(std::string devNameBase, PortList& portList) {

	//TODO: mutex here?
	// portList.clear();

	std::string path = "/dev";

	const std::regex expr(devNameBase + "\\d");

	/* list files on /dev and filter with the provided name base*/
	for (const auto& file : std::filesystem::directory_iterator(path)) {

		if (std::regex_match(file.path().string(), expr)) {
		 	LOG_S(9) << "Port found: " << file.path();
			portList.push_back(file.path());	 
		}
	}
}

bool isUthing(const PortName& fileDescriptor, PortObject& port) {
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
			the port is open, a few of data messages will be already queued.
		To overcome these situations, why we need to try the request a few times.
	*/
	for (int i = 0; i < 5; ++i)
	{
		port.FlushIOBuffers();	
		std::this_thread::sleep_for(10ms);
		LOG_S(9) << "Request {info}";		
		port.Write("{\"info\": true}\n");
		// port.Write(uThingQueries.info);
		
		try {	
			std::string response;
			port.ReadLine(response, '\n', 1500);
			LOG_S(9) << "Read " << response;

			json jmesg = json::parse(response);
			LOG_S(9) << "JSON resp " << jmesg;	

			if (jmesg.contains("info")) {
				LOG_S(6) << jmesg["info"]["device"];
				// j_info = jmesg;
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

void monitorPortsThread(std::map<PortName, Uthing> devices, std::mutex& mutex_devices) {

	PortList tempPortList;
	
	while(true) {

		tempPortList.clear();
		findPorts("/dev/ttyACM", tempPortList);	
		findPorts("/dev/ttyUSB", tempPortList);

		/* iterate over the obtained port list */
		for (const auto& portName : tempPortList) {
			LOG_S(9) << "Fetching info from device on " << portName;

			/* first check if a device is not already created on this port */
			if (devices.find(portName) == devices.end()) {				
				PortObject tempPort;
				if (isUthing(portName, tempPort)) {
					LOG_S(INFO) << "new uThing detected at " << portName;
				
					Uthing uThing(portName, std::move(tempPort));
					
					std::lock_guard<std::mutex> lck(mutex_devices);				
					//key: portName, value: uThing object
					devices.emplace(portName, std::move(uThing));
				}
			} else {
				LOG_S(9) << "device at " << portName << " already created";
			}
		}
		std::this_thread::sleep_for(chrono::milliseconds(500));
	}
}


}//namespace ubridge