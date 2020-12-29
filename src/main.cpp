#include <iostream>
#include <chrono>
#include <iomanip>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#define LOGURU_WITH_STREAMS 1
#include <loguru/loguru.hpp>
#include <loguru/loguru.cpp>

#include <nlohmann/json.hpp>

// #include "MQ.h"
// #include "reqRepServer.h"

#include <nngpp/nngpp.h>
#include <nngpp/protocol/req0.h>
#include <nngpp/protocol/rep0.h>

#include "ubridge.h"

using namespace std;




int main(int argc, char *argv[])
{

	loguru::g_preamble_date = false;
	loguru::g_preamble_time = false;

	//TODO: get verbosity from argv...
	loguru::g_stderr_verbosity = 6;
	loguru::init(argc, argv);

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
	using namespace ubridge;

	Bridge app;
	app.start();

	// nng::socket rep_sock = nng::rep::open();
	// rep_sock.listen("ipc:///ubridge/configSock");

	// nng::socket rep_socket = nng::rep::open();
	// rep_socket.listen( "ipc://ubridgeConf" );
	
	while(true){
		this_thread::sleep_for(chrono::milliseconds(1000));
		// LOG_S(INFO) << "-";
	}
		

	// delete msgQ;
	return 0;
}
