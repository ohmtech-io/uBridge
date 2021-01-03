#pragma once

#include <functional>
#define LOGURU_WITH_STREAMS 1
#include <loguru/loguru.hpp>
#include <nlohmann/json.hpp>
#include <nngpp/nngpp.h>
#include <nngpp/protocol/pub0.h>
#include <nngpp/platform/platform.h>
#include <cstdio>
#include <thread>

using namespace std;
// for convenience
using json = nlohmann::json;


class Streamer {
public:
	Streamer(const char* url) {

		// create a socket for the PUB protocol
		pub_sock = nng::pub::open();

		streamSockUrl = url;
	}

	~Streamer() {
	};

	int start() {
		try {
			/* PUB socket starts listening */
			pub_sock.listen(streamSockUrl);	
		} catch( const nng::exception& e ) {
			LOG_S(WARNING) << "nng Exception: " << e.who() << e.what();			
			return -1;
		}	

		LOG_S(INFO) << "Streamer (PUB socket) initialized on " << streamSockUrl;

		// std::thread _receiveThread(&ReqRepServer::receiveThread, this);
		// _receiveThread.detach();
		
		return 0;
	};

	int publish(std::string& topic, json& jmessage) {
	// int publish(json jmessage) {
		LOG_S(5) << "PUB -topic: " <<topic << " msg: " << jmessage;
		// std::string topic = "/sensors/3/data";
		auto msg = topic + "#" + jmessage.dump();

		pub_sock.send({msg.c_str(), msg.size()});
		return 0;
	}

	// void sendResponse(const std::string rawMsg) {		
		
	// 	try {
	// 		LOG_S(INFO) << "Send response: " << rawMsg;

	// 		auto msg = nng::make_msg(0);

	// 		//TODO: this is hacky, find the proper way to pass the string by reference on the msg constructor
	// 		msg.body().append({rawMsg.c_str(), rawMsg.length()});

	// 		rep_sock.send( std::move(msg) );

	// 	} catch( const nng::exception& e ) {
	// 		LOG_S(WARNING) << "nng Exception: " << e.who() << e.what();			
	// 	}
	// }

protected:
	// void receiveThread() {
	// 	LOG_S(INFO) << "listening for requests..";
	// 	while (true) {
	// 		// rep receives a message (blocking)
	// 		//tesing: nngcat --req --dial ipc:///tmp/ubridgeConf --data "{\"hello\":1}";
	// 		auto message = rep_sock.recv_msg();
	// 		auto body = message.body().data<char>();
	// 		LOG_S(INFO) << "Received request: " << body;	

	// 		json jmessage;
	// 		if (0 == parseMessage(body, jmessage)){
	// 			_jsonCb(jmessage);	
	// 		}

	// 		LOG_S(8) << "listening new requests";
	// 	}
	// }

	// int parseMessage(string message, json& jrecv){
	// 	try{
	// 		jrecv = json::parse(message);
	// 		LOG_S(5) << "Rx parsed JSON: " << std::setw(2) << jrecv;
	// 		return 0;
	// 	} catch (json::exception& e) {
	// 		LOG_S(WARNING) << "Error parsing JSON: " << e.what();
	// 		return -1;
	// 	}
	// }

public: 
	const char* streamSockUrl;

private:
	nng::socket pub_sock;
	// function<void(string&)> _rawCb;
	// function<void(json&)> _jsonCb;
};
