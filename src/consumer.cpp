#include <iostream>
#include <chrono>
#include <iomanip>

#define LOGURU_WITH_STREAMS 1
#include <loguru/loguru.hpp>
#include <loguru/loguru.cpp>

#include <nlohmann/json.hpp>

#include <nngpp/nngpp.h>
#include <nngpp/protocol/rep0.h>
#include <nngpp/platform/platform.h>

// #include "MQ.h"
#include "uBridgeConfig.h"
#include "reqRepClient.h"

// for convenience
using json = nlohmann::json;

using namespace std;

ubridge::Config cfg;

void subsMessageHandler(ubridge::message& message) {
	LOG_S(9) << "cb msg" << message.topic << message.data ;
}

int main(int argc, char *argv[])
{
	
	int verb_level = 0;

	if (argc > 1) { 
		verb_level = atoi(argv[1]);
	}
	loguru::g_stderr_verbosity = verb_level;
	
	loguru::g_preamble_date = false;
	loguru::g_preamble_time = false;
	// loguru::init(argc, argv);

	ReqRepClient client(cfg.configSockUrl, cfg.streamSockUrl);


	LOG_S(INFO) << "*** Starting client... ***" ;
	
	LOG_S(INFO) << "Pinging server..." ;
	if (client.connect() != 0) {
		/* wait for ubridge config server */
		LOG_S(INFO) << "Server not found!" ;	
		LOG_S(INFO) << "Waiting for server..." ;	
		while (client.connect() !=0) {
			this_thread::sleep_for(chrono::milliseconds(1000));
		}
	}
		
	json deviceList;

	client.getDevices(deviceList);

	LOG_S(INFO) << deviceList["devCount"] << " devices detected. Details:" << std::setw(2) << deviceList["devices"];	
	// ubridge::config cfg;
	// cfg.maxDevices = 2;
	// json jsoncfg = cfg;
	
	json query = "{\"status\":\"1\"}"_json;
	json command = "{\"led\":true}"_json;
	// json query = "{\"led\":false}"_json;
	json resp;

	std::string chID = "uThing::VOC_9142";
	client.queryDeviceById(chID, query, resp);

	client.sendCommand(chID, command, resp);
	//start message receiving loop...
	client.subscribe("/sensors", subsMessageHandler); //subscribe to all sensors
	// client.subscribe("/sensors/uThing::VOC_9142", subsMessageHandler); //specific one

    // json jsoncfg;
    // jsoncfg["maxDevices"] = 3;
    //or
    // jsoncfg = {{"maxDevices", 3}};
    // jsoncfg = {{"maxDevices", 3}, {"devNameBase", "/dev/andaadormir"}};

	return 0;
}
