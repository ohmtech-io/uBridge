#pragma once

#include <functional>
#define LOGURU_WITH_STREAMS 1
#include <loguru/loguru.hpp>
#include <nlohmann/json.hpp>
#include <nngpp/nngpp.h>
#include <nngpp/protocol/req0.h>
#include <cstdio>
#include <thread>


using namespace std;
// for convenience
using json = nlohmann::json;


class ReqRepClient {
public:
	ReqRepClient(const char* url) {

		// create a socket for the req protocol
		req_sock = nng::req::open();

		rrSockUrl = url;

		// _jsonCb = cb;
	}

	~ReqRepClient() {
		// delete rrMqsQ;
	};

	int connect() {
		try {
			
			/* REQ dials and establishes a connection */
			req_sock.dial(rrSockUrl);	

			req_sock.send("{\"ping\":0}");
			
			auto buf = req_sock.recv();

			auto messageRaw = buf.data<char>();

			LOG_S(INFO) << "received response: " << messageRaw;

		} catch( const nng::exception& e ) {
			LOG_S(WARNING) << "nng Exception: " << e.who() << e.what();			
			return 1;
		}	

		// LOG_S(INFO) << "Request socket connected";

		// std::thread _receiveThread(&ReqRepServer::receiveThread, this);
		// _receiveThread.detach();
		
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
			auto message = req_sock.recv_msg();
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
	const char* rrSockUrl;

private:
	nng::socket req_sock;
	function<void(string&)> _rawCb;
	function<void(json&)> _jsonCb;
};