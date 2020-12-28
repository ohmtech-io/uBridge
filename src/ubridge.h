#pragma once

#include <filesystem>
#include <regex>
#include <nlohmann/json.hpp>

#include "reqRepServer.h"

// for convenience
using json = nlohmann::json;

namespace ubridge {
	class config {
		public:
			std::string appVersionStr = "0.1";
	        std::string devNameBase = "/dev/ttyACM";
	        int maxDevices = 10;
	        const char* configSockUrl= "ipc:///tmp/ubridgeConf";
	};
	/*https://nlohmann.github.io/json/features/arbitrary_types/
	this enable us to convert the class to json (json jcfg = config)
	*/
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(config, appVersionStr, devNameBase, maxDevices)

	enum requestType_t {
		setConfig,
		getConfig,
		getChannels,
		ping,
		unrecognized,
	};
}

namespace ubridge {

class Bridge {
public:
	Bridge(void) {
		/* Calling with object non-static member function requires to bind the implicit "this" pointer.*/
		rrServer = new ReqRepServer(cfg.configSockUrl, 
							std::bind(&Bridge::cfgHandler, 
							this, 
							std::placeholders::_1));
	}

	~Bridge(){};

	void start(void) {
		rrServer->start();
	}

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

	void sendResponse(requestType_t& requestType) {
		std::string response;
		json jcfg = cfg;

		switch(requestType) {
			case ping:
				response = "{\"pong\":1}";
				break;
			case getConfig:
				findPorts(cfg.devNameBase, portList);
				// LOG_S(6) << "Available ports: " << portList.front();
				response = jcfg.dump();
				break;
			case setConfig:
				response = "{\"status\":\"OK\"}";
				break;
			case getChannels:
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
private:
	ReqRepServer *rrServer;
	requestType_t requestType;
	std::vector<std::string> portList;
};

} //namespace ubridge