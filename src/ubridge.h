#pragma once

#include <filesystem>
#include <regex>
#include <nlohmann/json.hpp>

#include "reqRepServer.h"
#include "uStreamer.h"
#include "uBridgeConfig.h"

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

	~Bridge(){};

	void start(void) {
		rrServer->start();
		uStreamer->start();
	}

	void publish(int& count) {uStreamer->publish(count);}

private:
	void findPorts(std::string &devNameBase, std::vector<std::string> &portList) {

		//TODO: mutex here
		portList.clear();

		std::string path = "/dev";

		const std::regex expr(devNameBase + "\\d");

		/* list files on /dev and filter with the provided name base*/
		for (const auto & file : std::filesystem::directory_iterator(path)) {

			if (std::regex_match(file.path().string(), expr)) {
			 	LOG_S(6) << "Port found: " << file.path();
				portList.push_back(file.path());	 
			}
		}
	}

	int listDevices() {
		int devCount = 0;

		findPorts(cfg.devNameBase, portList);

		devCount = portList.size();
		//WARNING: trying to print an empty vector causes segfault..
				// LOG_S(6) << "Available ports: " << portList.front();


		//TODO:
		/*
			- iterate the list, try connecting and query ({"info":true})
			- populate a json list with the json response
			- if device does't response, or get error, flag the port name?
			- mantain a list of connected devices (ports), what do we do it we 
			get a disconnection (how we notice a disconnection)?
		*/
		deviceList["devCount"] = devCount;

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

		// deviceList["devices"][1] = {"uTHingMNL":1};

		LOG_S(5) << "Device list: " << deviceList;

		return devCount;
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
private:
	ReqRepServer *rrServer;
	Streamer *uStreamer;
	requestType_t requestType;
	std::vector<std::string> portList;
};

} //namespace ubridge