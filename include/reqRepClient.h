#pragma once

#include <functional>
#define LOGURU_WITH_STREAMS 1
#include <loguru/loguru.hpp>
#include <nlohmann/json.hpp>
#include <nngpp/nngpp.h>
#include <nngpp/protocol/req0.h>
#include <nngpp/protocol/sub0.h>
#include <cstdio>
#include <thread>


// using namespace std;
// for convenience
using json = nlohmann::json;


class ReqRepClient {
public:
	ReqRepClient(const char* urlConf, const char* urlPubSub) {

		// create a socket for the REQ protocol
		req_sock = nng::req::open();

		reqSockUrl = urlConf;

		// create a socket or the SUB protocol
		sub_sock = nng::sub::open();

		streamSockUrl = urlPubSub;
	}

	~ReqRepClient() {
		// delete rrMqsQ;
	};

	int connect() {
		try {
			/* REQ dials and establishes a connection */
			req_sock.dial(reqSockUrl);	

			/* SUB dials and establishes a connection */
			// sub_sock.dial(reqSockUrl);	

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

	int subscribe(const char* topic) {

					/*
			 // subscribe to everything (empty means all topics)
        if ((rv = nng_setopt(sock, NNG_OPT_SUB_SUBSCRIBE, "", 0)) != 0) {
                fatal("nng_setopt", rv);
        }
        if ((rv = nng_dial(sock, url, NULL, 0)) != 0) {
                fatal("nng_dial", rv);
        }
        */

		/* Look here for the all subscription: https://github.com/cwzx/nngpp/issues/9 
		https://github.com/cwzx/nngpp/issues/21
		*/
		// subscribe to everything (empty means all topics)
		sub_sock.set_opt(NNG_OPT_SUB_SUBSCRIBE, {});

		// sub_sock.set_opt( NNG_OPT_SUB_SUBSCRIBE, nng::view("",0) );
		
		sub_sock.dial(reqSockUrl);	

		// LOG_S(5) << "Listening..";
		LOG_S(INFO) << "Subscribing to messages on " << streamSockUrl;
		// sub_socket.set_opt_string
		while(true){
			try {
				LOG_S(5) << "Listening..";
				auto buf = sub_sock.recv(NNG_FLAG_ALLOC);
				// auto buf = sub_sock.recv(NNG_FLAG_NONBLOCK);
				LOG_S(5) << "...";
				auto messageRaw = buf.data<char>();
				LOG_S(5) << "received response: " << messageRaw;
				} catch( const nng::exception& e ) {
					LOG_S(WARNING) << "nng Exception: " << e.who() << e.what();			
					return -1;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
	}

protected:
	int parseMessage(std::string message, json& jrecv){
		/* convert to JSON */
		try{
			jrecv = json::parse(message);
			LOG_S(6) << "Rx parsed JSON: " << std::setw(2) << jrecv;
			return 0;
		} catch (json::exception& e) {
			LOG_S(WARNING) << "Error parsing JSON: " << e.what();
			return -1;
		}
	}

public: 
	const char* reqSockUrl;
	const char* streamSockUrl;

private:
	nng::socket req_sock;
	nng::socket sub_sock;

	std::function<void(std::string&)> _rawCb;
	std::function<void(json&)> _jsonCb;
};