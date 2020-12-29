#pragma once

#include <functional>
#define LOGURU_WITH_STREAMS 1
#include <loguru/loguru.hpp>
#include <nlohmann/json.hpp>
#include <nngpp/nngpp.h>
#include <nngpp/protocol/req0.h>
#include <cstdio>
#include <thread>


// using namespace std;
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
		} catch( const nng::exception& e ) {
			LOG_S(1) << "nng Exception: " << e.who() << e.what();			
			return 1;
		}	

		try {
			LOG_S(5) << "Send {\"ping\"} request.";		
			req_sock.send("{\"ping\":0}");
			
			auto buf = req_sock.recv();

			auto messageRaw = buf.data<char>();
			LOG_S(5) << "received response: " << messageRaw;
		} catch( const nng::exception& e ) {
			LOG_S(WARNING) << "nng Exception: " << e.who() << e.what();			
			return -1;
		}	
		LOG_S(INFO) << "Server OK";

		// std::thread _receiveThread(&ReqRepServer::receiveThread, this);
		// _receiveThread.detach();
		
		return 0;
	}

	int getDevices(json &deviceList) {
		try {
			LOG_S(5) << "Request connected devices.";		
			req_sock.send("{\"command\":\"getDevices\"}");
			
			auto buf = req_sock.recv();

			auto messageRaw = buf.data<char>();
			LOG_S(5) << "received response: " << messageRaw;

			if (0 == parseMessage(messageRaw, deviceList)){
				return 0;
			}
		} catch( const nng::exception& e ) {
			LOG_S(WARNING) << "nng Exception: " << e.who() << e.what();			
			return -1;
		}
		return -1;
	}

protected:
	int parseMessage(std::string message, json& jrecv){
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
	std::function<void(std::string&)> _rawCb;
	std::function<void(json&)> _jsonCb;
};