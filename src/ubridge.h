#pragma once

#include <filesystem>
#include <regex>
#include <nlohmann/json.hpp>

#include "threadsafeQueue.h"
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

		inboundQ.setMaxSize(100);

		LOG_S(INFO) << "Starting monitor thread";
		// std::thread monitorPorts(&monitorPortsThread, std::ref(devices), std::ref(mutex_devices), std::ref(cfg));
		
		std::thread monitorPorts(&monitorPortsThread, this);
		monitorPorts.detach();
		//DM testing
		// json newMsg = inboundQ.pop();
		// LOG_S(INFO) << "newMessageBridge" << newMsg;
		json inMessage;
		const std::string base_topic = "/sensors/";

		while (1) {
			inMessage = inboundQ.pop();
			try {
				LOG_S(8) << "Publishing data: " << inMessage;
	

				std::string topic = base_topic + inMessage["channelID"].get<std::string>();
				inMessage.erase("channelID");

				publish(topic, inMessage);
			}
			catch (const json::exception& e) {
				LOG_S(WARNING) << "Error parsing JSON: " << e.what();
			}
			std::this_thread::sleep_for(5ms);
		}
	}

	//temp, testing
	void publish(std::string& topic, json& jmessage) {uStreamer->publish(topic, jmessage);}	

private:
	int listDevices() {

		//this creates an array stored as std::vector
		deviceList["devices"] = {};

		const std::lock_guard<std::mutex> lck(mutex_devices);				

		for (auto& [port, uthing] : devices) {
			json jdevice;
			jdevice["name"] = uthing.devName();
			jdevice["channelID"] = uthing.channelID();
			jdevice["serialNumber"] = uthing.serialNumber();
			jdevice["fwVersion"] = uthing.fwVersion();

			deviceList["devices"].push_back(jdevice);
		}
		
		deviceList["devCount"] = devices.size();
		LOG_S(5) << "Device list: " << deviceList;

		/*
		{
 	   "info": {
    	    "device": "uThing::VOC rev.A",
        	"serial": "27D521601B89CE9A",
        	"firmware": "1.2.1"
    		}
		}
		*/

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
				LOG_S(WARNING) << "TODO: implement this";
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

	int parseCommand(const json& jmessage) {
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
	Config cfg;	
	json deviceList;
	// std::vector<Uthing> devices;
	std::map<PortName, Uthing> devices;
	std::mutex mutex_devices;

	TQueue<json> inboundQ;
	TQueue<json> outboundQ;

private:
	// USerial uSerial;
	ReqRepServer *rrServer;
	Streamer *uStreamer;
	requestType_t requestType;
	PortList portsNameList;
}; //class Bridge 
} //namespace ubridge