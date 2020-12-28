#include <iostream>
#include <chrono>
#include <iomanip>

#define LOGURU_WITH_STREAMS 1
#include <loguru/loguru.hpp>
#include <loguru/loguru.cpp>

#include <nlohmann/json.hpp>

#include <nngpp/nngpp.h>
#include <nngpp/protocol/rep0.h>

// #include "MQ.h"
#include "ubridge.h"
#include "reqRepClient.h"

// for convenience
using json = nlohmann::json;

ubridge::config cfg;


int main(int argc, char *argv[])
{
	loguru::g_stderr_verbosity = 5;
	loguru::g_preamble_date = false;
	loguru::g_preamble_time = false;
	// loguru::init(argc, argv);

	ReqRepClient client(cfg.configSockUrl);


	LOG_S(INFO) << "Start clienting - Config ch: " ;
	client.connect();
	/* wait for ubridge config server */

	// ubridge::config cfg;
	// cfg.maxDevices = 2;
	// json jsoncfg = cfg;
	// msgQ->sendMessage(jsoncfg.dump());

    json jsoncfg;
    // jsoncfg["maxDevices"] = 3;
    //or
    jsoncfg = {{"maxDevices", 3}};
    // jsoncfg = {{"maxDevices", 3}, {"devNameBase", "/dev/andaadormir"}};

    /* serialize and send */



	return 0;
}
