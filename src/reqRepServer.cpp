#include <functional>
#define LOGURU_WITH_STREAMS 1
#include <loguru/loguru.hpp>
#include <nlohmann/json.hpp>
#include <nngpp/nngpp.h>
#include <nngpp/protocol/rep0.h>
#include <cstdio>
#include <thread>
#include <iomanip>

#include "reqRepServer.h"

using namespace std;
// for convenience
using json = nlohmann::json;


ReqRepServer::ReqRepServer(std::string& url, function<void(json&)> cb) {

	// create a socket for the rep protocol
	rep_sock = nng::rep::open();

	rrSockUrl = url;
	_jsonCb = cb;
}

int ReqRepServer::start() {
	try {
		/* REP socket starts listening */
		rep_sock.listen(rrSockUrl.c_str());	
	} catch( const nng::exception& e ) {
		LOG_S(WARNING) << "nng Exception: " << e.who() << e.what();			
		// printf( "%s: %s\n", e.who(), e.what() );
		return -1;
	}	

	LOG_S(INFO) << "Config Server listening on: " << rrSockUrl;

	std::thread _receiveThread(&ReqRepServer::receiveThread, this);
	_receiveThread.detach();
	
	return 0;
}

void ReqRepServer::sendResponse(const std::string rawMsg) {		
	
	try {
		auto msg = nng::make_msg(0);

		//TODO: this is hacky, find the proper way to pass the string by reference on the msg constructor
		msg.body().append({rawMsg.c_str(), rawMsg.length()});

		LOG_S(INFO) << "Send response: " << rawMsg << " lenght: " << msg.body().size();
		rep_sock.send( std::move(msg) );

	} catch( const nng::exception& e ) {
		LOG_S(WARNING) << "nng Exception: " << e.who() << e.what();			
	}
}


void ReqRepServer::receiveThread() {
	LOG_S(INFO) << "listening for requests..";
	while (true) {
		// rep receives a message (blocking)
		//tesing: nngcat --req --dial ipc:///tmp/ubridgeConf --data "{\"hello\":1}";
		auto message = rep_sock.recv_msg();
		auto body = message.body().data<char>();
		LOG_S(INFO) << "Received request: " << body;	

		json jmessage;
		if (0 == parseMessage(body, jmessage)){
			_jsonCb(jmessage);	
		}

		LOG_S(8) << "listening new requests";
	}
}

int ReqRepServer::parseMessage(string message, json& jrecv){
	try {
		jrecv = json::parse(message);
		LOG_S(5) << "Rx parsed JSON: " << std::setw(2) << jrecv;
		return 0;
	} catch (json::exception& e) {
		LOG_S(WARNING) << "Error parsing JSON: " << e.what();
		return -1;
	}
}
