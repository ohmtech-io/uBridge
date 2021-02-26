#include <regex>
#include <filesystem>
#include <thread>
#include <chrono>

#include <libserial/SerialPort.h>
#include <nlohmann/json.hpp>
#define LOGURU_WITH_STREAMS 1
#include <loguru/loguru.hpp>

#include "ubridge.h"
#include "uSerial.h"

namespace ubridge {
// for convenience
using json = nlohmann::json;
using namespace std::chrono_literals;

void findPorts(std::string devNameBase, PortList& portList) {

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
	} 
	catch (const std::exception& ex) {
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
		
		try {	
			std::string response;
			port.ReadLine(response, '\n', 1500);
			LOG_S(9) << "Read " << response;

			json jmesg = json::parse(response);
			LOG_S(9) << "JSON resp " << jmesg;	

			if (jmesg.contains("info")) {
				LOG_S(9) << jmesg["info"]["device"];
				// info = jmesg;
				return true;
			}			
		} 
		catch (const std::exception& ex) {
			LOG_S(WARNING) << fileDescriptor << ": " << ex.what();
		}

		std::this_thread::sleep_for(10ms);
	}
	//couldn't get the info (not a uThing device probably?)	
	return false;
}

// void assignChannelID( Uthing& uthing, Bridge* bridge) {
// 	//-----"device": "uThing::VOC rev.A"----
// 	 std::string devFullName = uthing.devName();
// 	 //remove " rev.X"
// 	std::string devName = devFullName.substr(0, devFullName.size() - 6);
// 	std::string serial = uthing.serialNumber();

// 	uthing.setChannelID(devName + '_' + serial.substr(serial.length()-4, serial.length()));
	
// 	LOG_S(6) << "dev name: " << uthing.devName() <<", ChannelID: " << uthing.channelID();
// }

void monitorPortsThread(Bridge* bridge) {
	PortList tempPortList;

	loguru::set_thread_name("monitorPorts");
	
	if (bridge->cfg.devNameBase.empty()) {
		LOG_S(INFO) << "Searching for devices on /dev/ttyACM* and /dev/ttyUSB*";
	} else {
		LOG_S(INFO) << "Searching for devices on " << bridge->cfg.devNameBase;
	}

	while(true) {

		tempPortList.clear();

		if (bridge->cfg.devNameBase.empty()) {
			findPorts("/dev/ttyACM", tempPortList);	
			findPorts("/dev/ttyUSB", tempPortList);
		} else {
			findPorts(bridge->cfg.devNameBase, tempPortList);
		}
		/* iterate over the obtained port list */
		for (const auto& portName : tempPortList) {

			/* first check if a device is not already created on this port */
			if (bridge->devices.find(portName) == bridge->devices.end()) {				
				PortObject tempPort;
				json info;

				LOG_S(5) << "Fetching info from device on " << portName;
				if (isUthing(portName, tempPort)) {
					LOG_S(INFO) << "new uThing detected at " << portName;
				
					Uthing uThing(portName, std::move(tempPort));
					uThing.assignChannelID();
					
					{
						const std::lock_guard<std::mutex> lck(bridge->mutex_devices);				
						//key: portName, value: uThing object
						bridge->devices.emplace(portName, std::move(uThing));
					
						// LOG_S(INFO) << bridge->devices.find(portName)->second.info();
						//create a thread from a member function, with the instance stored on the map, pass the bridge instance too
						std::thread relayThread(&Uthing::relayThread, &bridge->devices.find(portName)->second, std::ref(*bridge));
						relayThread.detach();
					}
				}
			} else {
				LOG_S(9) << "device at " << portName << " already created";
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}//monitor loop
}

}//namespace ubridge
