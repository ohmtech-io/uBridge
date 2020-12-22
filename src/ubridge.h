#pragma once

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
	};
	/*https://nlohmann.github.io/json/features/arbitrary_types/*/
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(config, appVersionStr, devNameBase, maxDevices)

}

namespace ubridge {

class Ubridge {
public:
	Ubridge(std::string& cfgCHname ) {
		rrServer = new ReqRepServer(cfgCHname, std::bind(&Ubridge::cfgHandler, this, std::placeholders::_1));
	}

	~Ubridge(){};

	void start(void) {
		rrServer->start();
	}


private:
	void cfgHandler(json& jmessage){
		LOG_S(6) << "confHandler> JSON message: " << jmessage;

		/*we can't use the arbitrary_types feature to fill the config here, if one key is missing, or doesn't match, it fails*/
		if (jmessage.contains("maxDevices")) cfg.maxDevices = jmessage["maxDevices"];
		if (jmessage.contains("devNameBase")) cfg.devNameBase = jmessage["devNameBase"];

		/* just checking the results..*/
		json j = cfg;
		LOG_S(INFO) << "actual config: " << j;

		std::string msg = "{\"status\":\"OK\"}";

		// rrServer->sendResponse(msg);
	}


public:
	config cfg;	
private:
	ReqRepServer *rrServer;
};

} //namespace ubridge