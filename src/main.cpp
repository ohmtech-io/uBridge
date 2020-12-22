#include <iostream>
#include <chrono>
#include <iomanip>

#define LOGURU_WITH_STREAMS 1
#include <loguru/loguru.hpp>
#include <loguru/loguru.cpp>

#include <nlohmann/json.hpp>

#include "MQ.h"
#include "reqRepServer.h"
#include "ubridge.h"


using namespace std;

// ubridge::config cfg;

// void cfgHandler(json& jmessage);
/*note: the messageQueue name defined here has to be consistent in the client */
// ReqRepServer rrServer("configCH", &cfgHandler);


// const std::string sampleMessg = "{\"temperature\": 24.68, \"pressure\": 1019.38, \"humidity\": 45.64, \"gasResistance\": 597617, \"IAQ\": 28.3, \"iaqAccuracy\": 3, \"eqCO2\": 511.21, \"eqBreathVOC\": 0.52}";

// void cfgHandler(json& jmessage){
// 	LOG_S(6) << "confHandler> JSON message: " << jmessage;

// 	/*we can't use the arbitrary_types feature to fill the config here, if one key is missing, or doesn't match, it fails*/
// 	if (jmessage.contains("maxDevices")) cfg.maxDevices = jmessage["maxDevices"];
// 	if (jmessage.contains("devNameBase")) cfg.devNameBase = jmessage["devNameBase"];

// 	/* just checking the results..*/
// 	json j = cfg;
// 	LOG_S(INFO) << "actual config: " << j;

// 	rrServer.rrSendResponse("{\"status\":\"OK\"}");
// }

int main(int argc, char *argv[])
{
	// MQ *msgQ;

	loguru::g_preamble_date = false;
	loguru::g_preamble_time = false;
	loguru::init(argc, argv);

	// bool client1_Subs = false;

	LOG_S(INFO) << "--- Initializing **u-bridge**... ---";

	
	
	// cfg.maxDevices = 5;
	
	/* use the built-in conversion thanks to the NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE macro*/
	// json jcfg = cfg;
	
	// convert to JSON: copy each value into the JSON object (manually)
	// jcfg["chName"] = cfg.chName;
	// jcfg["lalala"] = cfg.lalala;
	// jcfg["maxDevices"] = cfg.maxDevices;

	// // LOG_S(INFO) << std::setw(2) << jcfg;
	// LOG_S(INFO) << jcfg;


	// conversion: json -> struct
	// auto jc2 = jcfg.get<ubridge::config>();
	// LOG_S(INFO) << jc2.chName;
	// json j2;
	// j2 = json::parse(sampleMessg);

	//  std::cout << std::setw(2) << j2 << '\n';

	//  std::cout << j2["IAQ"] << '\n';

	// msgQ = new MQ("ubridge", MQ::EndpointType::Server);
	// msgQ->listen([msgQ, &client1_Subs](string message){
	// 	LOG_S(INFO) << "Received: " << message;
		
	// 	if (message == "/sensor?") {
	// 		msgQ->sendMessage("Subscription registered!");
	// 		client1_Subs = true;
	// 	}
	// });

	// while(true) {
	// 	if (client1_Subs){
	// 		msgQ->sendMessage(sampleMessg);
	// 	}
	// 	this_thread::sleep_for(chrono::milliseconds(100));
	// }
	using namespace ubridge;

	string cfgCHname = "configCH";
	Ubridge ubridge(cfgCHname);
	ubridge.start();

	while(true){
		this_thread::sleep_for(chrono::milliseconds(100));
	}
		

	// delete msgQ;
	return 0;
}
