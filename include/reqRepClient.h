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
			LOG_S(INFO) << "Connecting to REQ/REP socket: " << reqSockUrl;
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

		LOG_S(INFO) << "Listening on socket: " << streamSockUrl;

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

	int subscribe(const char* topic, std::function<void(ubridge::message&)> cb) {

		/* Look here for the all subscription: 
			https://github.com/cwzx/nngpp/issues/9 
			https://github.com/cwzx/nngpp/issues/21
		*/
		/* use nullptr to subscribe to all*/
 		// sub_sock.set_opt(NNG_OPT_SUB_SUBSCRIBE, "/sensors/1");

		// using namespace ubridge;

 		_jsonCb = cb;
 		
 		// std::string rec_topic;
 		// json rec_json;

 		size_t lengthWithoutNull = strlen(topic)-1;
 		char topicChArray[lengthWithoutNull];

 		for (int i = 0; i < lengthWithoutNull; ++i)
 		{
 			topicChArray[i] = topic[i];
 		}
		
 		sub_sock.set_opt(NNG_OPT_SUB_SUBSCRIBE, {topicChArray, lengthWithoutNull});
 		
		sub_sock.dial(streamSockUrl);	

		LOG_S(INFO) << "Subscribing to messages on " << topic;

		while(true){
			try {
				
				nng::buffer buf = sub_sock.recv(NNG_FLAG_ALLOC);
				std::string messageRaw = buf.data<char>();


				LOG_S(7) << "received: " << messageRaw;

				ubridge::message message;
				splitMessage(messageRaw, message.topic, message.data);

				LOG_S(5) << "Topic " << message.topic;
				LOG_S(5) << "Data " << message.data;

			
				_jsonCb(message);	
			

				} catch( const nng::exception& e ) {
					LOG_S(WARNING) << "nng Exception: " << e.who() << e.what();			
					return -1;
			}
		}
	}

protected:
	void splitMessage(const std::string& msg, std::string& topic, json& jdata) {
		/* we use # as token to separate topics from data */
		std::size_t pos = msg.find("#"); 

		topic = msg.substr(0, pos); 

		std::string data = msg.substr(pos+1); 
		
		parseMessage(data, jdata);
	}

	int parseMessage(const std::string& msg, json& jrecv) {
		/* convert to JSON */
		try{
			jrecv = json::parse(msg);
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

	std::function<void(ubridge::message&)> _jsonCb;
};