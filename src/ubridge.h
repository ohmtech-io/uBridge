#pragma once

#include <filesystem>
#include <regex>
#include <nlohmann/json.hpp>

#include "reqRepServer.h"
#include "uStreamer.h"
#include "uBridgeConfig.h"
#include "uSerial.h"
#include "uDevice.h"


// for convenience
using json = nlohmann::json;

namespace ubridge {

class Bridge {
public:
	Bridge(void) {
		/* Calling with object non-static member function requires to bind the implicit "this" pointer.*/
		rrServer = new ReqRepServer(cfg.configSockUrl, 
							std::bind(&Bridge::cfgHandler, 
							this, 
							std::placeholders::_1));

		uStreamer = new Streamer(cfg.streamSockUrl);
	}

	~Bridge(){
		delete rrServer;
		delete uStreamer;
	};

	void start(void) {
		rrServer->start();
		uStreamer->start();
		LOG_S(INFO) << "Starting monitor thread";
		std::thread monitorPorts(monitorPortsThread, std::ref(devices), std::ref(mutex_devices));
		monitorPorts.join();
	}

	//temp, testing
	void publish(std::string& topic, json& jmessage) {uStreamer->publish(topic, jmessage);}	

private:
	int listDevices() {
		// size_t devCount = 0;
		size_t portsCount = 0;

		portsNameList.clear();
		findPorts("/dev/ttyACM"s, portsNameList);	
		findPorts("/dev/ttyUSB"s, portsNameList);	
		// findPorts(cfg.devNameBase, portsNameList);

		portsCount = portsNameList.size();
		//WARNING: trying to print an empty vector causes segfault..

		LOG_S(INFO) << portsCount << " serial ports detected";

		//this creates an array stored as std::vector
		deviceList["devices"] = {};
		/*
		{
 	   "info": {
    	    "device": "uThing::VOC rev.A",
        	"serial": "27D521601B89CE9A",
        	"firmware": "1.2.1"
    		}
		}
		*/
		/* iterate over the obtained port list */
		// for (const auto& portName : portsNameList) {
		// 	LOG_S(5) << "Fetching info from device on " << portName;

		// 	/* first check if a device is not already created on this port */
		// 	if (devices.find(portName) == devices.end()) {				
		// 		PortObject tempPort;
		// 		if (isUthing(portName, tempPort)) {
		// 			LOG_S(INFO) << "uThing detected at " << portName;
				
		// 			Uthing uThing(portName, std::move(tempPort));
					
		// 			//key: portName, value: uThing object
		// 			devices.emplace(portName, std::move(uThing));
		// 			++devCount;

		// 			// devices.push_back(std::move(uThing));
		// 			// devices.emplace({portName, std::move(uThing)})
		// 			json j_info = uThing.info();
		// 		}
		// 	} else {
		// 		LOG_S(INFO) << "device at " << portName << " already created";
		// 	}
		// }


// * issues:
// when we unplug a device, how do we detect it?
// when we plug it back, Linux can enumerate it witha different port name (ttyACM0 becomes ttyACM2 for instance)
// we will need to query and uniquely identify them by serial number....
		// for (size_t i = 0; i < portsCount; ++i)
		// {
		// 	portName_t portName = portsNameList[i];
		// 	LOG_S(5) << "Fetching info from device on " << portsNameList[i];
		// 	if (0 == serial->open(portName)) {
		// 		json info;

		// 		/* query the device 'info' */
		// 		// if (0 == queryInfo(portName, info)) {
		// 		// 	++devCount;

		// 		// 	json device;
		// 		// 	device["name"] = info.device; /*"device": "uThing::VOC rev.A"*/
		// 		// 	device["channelID"] = createChannelID(info);
		// 		// 	device["serialNumber"] = info.serial;

		// 		// 	deviceList["devices"].push_back(device);
		// 		// }

		// 		serial->close(portName);
		// 	}
		// }

		//TODO:
		/*
			- iterate the list, try connecting and query ({"info":true})
			- populate a json list with the json response
			- if device does't response, or get error, flag the port name?
			- mantain a list of connected devices (ports), what do we do it we 
			get a disconnection (how we notice a disconnection)?
		*/
		json dev1;
		dev1["name"] = "uThingMNL"; 
		dev1["channelID"] = "uThingMNL#694";
		dev1["upTime"] = 1324567; 
		deviceList["devices"].push_back(dev1); 

		json dev2;
		dev2["name"] = "uThingVOC"; 
		dev2["channelID"] = "uThingVOC#234";
		dev2["upTime"] = 147373; 

		deviceList["devices"].push_back(dev2);
		
		deviceList["devCount"] = devices.size();

		LOG_S(5) << "Device list: " << deviceList;

		return devices.size();
	}

	void sendResponse(requestType_t& requestType) {
		std::string response;
		json jcfg = cfg;

		switch(requestType) {
			case ping:
				response = "{\"pong\":1}";
				break;
			case getConfig:
				/* send the actual configuration converted to JSON*/
				response = jcfg.dump();
				break;
			case setConfig:
				response = "{\"status\":\"OK\"}";
				break;
			case getDevices:
				listDevices();
				response = deviceList.dump();
				break;
			case queryDevice:
				LOG_S(WARNING) << "TODO: implement this";
				break;
			case unrecognized:
				response = "{\"status\":\"ERROR\",\"error\":\"command not valid\"}";
				break;
		}

		rrServer->sendResponse(response);
	}

	int parseCommand(json& jmessage) {
		int ret = -1;
		
		if (jmessage["command"] == "setConfig") {
			/*we can't use the arbitrary_types feature to fill the config here, if one key is missing, or doesn't match, it fails*/
			if (jmessage.contains("maxDevices")) {
				//nngcat --req --dial ipc:///tmp/ubridgeConf --data "{\"command\":\"setConfig\",\"maxDevices\":1}";		
				requestType = setConfig;	
				cfg.maxDevices = jmessage["maxDevices"];
				ret = 0;
			} 
			if (jmessage.contains("devNameBase")) {
				requestType = setConfig;
				cfg.devNameBase = jmessage["devNameBase"];
				ret = 0;
			}
			/* just checking the results..*/
			json j = cfg;
			LOG_S(3) << "actual config: " << j;
		} 
		if (jmessage["command"] == "getConfig") {
			// nngcat --req --dial ipc:///tmp/ubridgeConf --data "{\"command\":\"getConfig\"}";
			requestType = getConfig;	
			ret = 0;
		}
		if (jmessage["command"] == "getDevices") {
			// nngcat --req --dial ipc:///tmp/ubridgeConf --data "{\"command\":\"getDevices\"}";
			requestType = getDevices;	
			ret = 0;
		}
		if (jmessage["command"] == "queryDevice") {
			// nngcat --req --dial ipc:///tmp/ubridgeConf --data "{\"command\":\"queryDevice\", \"status\":true}";
			requestType = queryDevice;	
			LOG_S(WARNING) << "TODO: implement this";
			ret = 0;
		}		
		return ret;
	}

	void cfgHandler(json& jmessage)	{
		LOG_S(6) << "confHandler> JSON message: " << jmessage;

		requestType = unrecognized;

		if (jmessage.contains("ping")) {
			requestType = ping;
		} else if (jmessage.contains("command")) {
			if (parseCommand(jmessage) != 0) {
				LOG_S(WARNING) << "Error parsing command!";
			}
		}
		sendResponse(requestType);
	}

public:
	config cfg;	
	json deviceList;
	// std::vector<Uthing> devices;
	std::map<PortName, Uthing> devices;
	std::mutex mutex_devices;


private:
	// USerial uSerial;
	ReqRepServer *rrServer;
	Streamer *uStreamer;
	requestType_t requestType;
	PortList portsNameList;
}; //class Bridge 
} //namespace ubridge