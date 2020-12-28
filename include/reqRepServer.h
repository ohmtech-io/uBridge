#pragma once

#include <functional>
#define LOGURU_WITH_STREAMS 1
#include <loguru/loguru.hpp>
#include <nlohmann/json.hpp>
#include <nngpp/nngpp.h>
#include <nngpp/protocol/rep0.h>
#include <cstdio>
#include <thread>

#include "MQ.h"

using namespace std;
// for convenience
using json = nlohmann::json;

// extern loguru::g_stderr_verbosity;


class ReqRepServer {
public:
	ReqRepServer(const char* url, function<void(json&)> cb) {

		// create a socket for the rep protocol
		rep_sock = nng::rep::open();

		// rrMqsQ = new MQ(name, MQ::EndpointType::Server);
		// rrMsgQname = name;

		rrSockUrl = url;

		_jsonCb = cb;
	}

	~ReqRepServer() {
		// delete rrMqsQ;
	};

	int start() {
		try {
			/* REP socket starts listening */
			rep_sock.listen(rrSockUrl);	
		} catch( const nng::exception& e ) {
			LOG_S(WARNING) << "nng Exception: " << e.who() << e.what();			
			// printf( "%s: %s\n", e.who(), e.what() );
			return 1;
		}	

		LOG_S(INFO) << "Config Server listening on: " << rrSockUrl;

		std::thread _receiveThread(&ReqRepServer::receiveThread, this);
		_receiveThread.detach();
		
		return 0;
	};

	void sendResponse(std::string rawMsg) {
		// rrMqsQ->sendMessage(rawMsg);
		LOG_S(5) << "Sending response " << rawMsg;
	} 

protected:
	void receiveThread() {
		LOG_S(INFO) << "listening for requests..";
		while (true) {
			// rep receives a message (blocking)
			// nng::buffer rep_buf = rep_sock.recv();
			// LOG_S(INFO) << "Received request: " << rep_buf;	

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

	int parseMessage(string message, json& jrecv){
		try{
			jrecv = json::parse(message);
			LOG_S(5) << "Rx parsed JSON: " << std::setw(2) << jrecv;
			return 0;
		} catch (json::exception& e) {
			LOG_S(WARNING) << "Error parsing JSON: " << e.what();
			return -1;
		}
	}

public: 
	// string rrMsgQname;
	const char* rrSockUrl;

private:
	// MQ *rrMqsQ;
	nng::socket rep_sock;
	function<void(string&)> _rawCb;
	function<void(json&)> _jsonCb;
};