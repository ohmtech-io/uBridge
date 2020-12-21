#include <iostream>
#include <chrono>

#include "MQ.h"
#include "reqRepServer.h"

#define LOGURU_WITH_STREAMS 1
#include <loguru.hpp>
#include <loguru.cpp>


using namespace std;

const std::string sampleMessg = "{\"temperature\": 24.68, \"pressure\": 1019.38, \"humidity\": 45.64, \"gasResistance\": 597617, \"IAQ\": 28.3, \"iaqAccuracy\": 3, \"eqCO2\": 511.21, \"eqBreathVOC\": 0.52}";

void handler(string& message){
	LOG_S(INFO) << "confHandler, message: " << message;
}

int main(int argc, char *argv[])
{
	// MQ *msgQ;

	loguru::init(argc, argv);

	// bool client1_Subs = false;

	LOG_S(INFO) << "--- Initializing **u-bridge**... ---";

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

	ReqRepServer rrServer("configCH", &handler);
	rrServer.start();

	while(true){
		this_thread::sleep_for(chrono::milliseconds(100));
	}
		

	// delete msgQ;
	return 0;
}
