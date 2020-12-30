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

ubridge::config cfg;


int main(int argc, char *argv[])
{
	loguru::g_stderr_verbosity = 5;
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
	// msgQ->sendMessage(jsoncfg.dump());


	client.subscribe("/sensors");

    json jsoncfg;
    // jsoncfg["maxDevices"] = 3;
    //or
    jsoncfg = {{"maxDevices", 3}};
    // jsoncfg = {{"maxDevices", 3}, {"devNameBase", "/dev/andaadormir"}};

	return 0;
}
