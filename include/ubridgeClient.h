/***************************************************************************
*** MIT License ***
*
*** Copyright (c) 2020-2021 Daniel Mancuso - OhmTech.io **
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.     
****************************************************************************/

#pragma once

#include <iostream>
#include <chrono>
#include <iomanip>
#include <functional>
#include <cstdio>
#include <thread>

#define LOGURU_WITH_STREAMS 1
#include <loguru/loguru.hpp>
#include <loguru/loguru.cpp>
#include <nlohmann/json.hpp>
#include <nngpp/nngpp.h>
#include <nngpp/protocol/req0.h>
#include <nngpp/protocol/sub0.h>
#include <nngpp/platform/platform.h>


// for convenience
using json = nlohmann::json;

class ReqRepClient {
public:
	ReqRepClient(std::string& urlConf, const char* urlPubSub) {

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
			req_sock.dial(reqSockUrl.c_str());	

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
		return 0;
	}

	int getDevices(json &deviceList) {
		LOG_S(5) << "Request connected devices.";		

		json req = "{\"request\":\"getDevices\"}"_json;
		
		if (-1 == request(req, deviceList)) {
			LOG_S(WARNING) << "Error requesting devices";
			return -1;
		} else return 0;
	}

	int getStatistics(json &response) {
		LOG_S(5) << "Request Statistics";		

		json req = "{\"request\":\"getStatistics\"}"_json;
		
		if (-1 == request(req, response)) {
			LOG_S(WARNING) << "Error requesting Statistics";
			return -1;
		} else return 0;
	}

	int sendCommand(std::string channelID, json& jrequest, json& response) {
		json req;

		req["request"] = "sendCommand";
		req["channelID"] = channelID;
		req["command"] = jrequest;

		LOG_S(5) << "Send Command: "<< req;

		if (-1 == request(req, response)) {
			LOG_S(WARNING) << "Error sending command to device";
			return -1;
		}  
		LOG_S(5) << "Command response: "<< response;
		return 0;
	}

	int queryDeviceById(std::string channelID, json& jrequest, json& response) {
		//NOTE that the uTHing devices always respond with a status message after a command
		//i.e.: {"status":{"format":"JSON","reportingPeriod":3,"temperatureOffset:":3.0,"upTime":10389561}}
		json req;

		req["request"] = "queryDevice";
		req["channelID"] = channelID;
		req["query"] = jrequest;

		LOG_S(5) << "Req.Query: "<< req;

		if (-1 == request(req, response)) {
			LOG_S(WARNING) << "Error querying device";
			return -1;
		}  
		LOG_S(5) << "Query response: "<< response;
		return 0;	
	}

	int subscribe(const char* topic, std::function<void(ubridge::message&)> cb) {

		/* Look here for the all subscription: 
			https://github.com/cwzx/nngpp/issues/9 
			https://github.com/cwzx/nngpp/issues/21
		*/
		/* use nullptr to subscribe to all*/
 		// sub_sock.set_opt(NNG_OPT_SUB_SUBSCRIBE, "/sensors/1");
 		_jsonCb = cb;
 	
 		size_t lengthWithoutNull = strlen(topic)-1;
 		char topicChArray[lengthWithoutNull];

 		for (int i = 0; i < lengthWithoutNull; ++i)
 		{
 			topicChArray[i] = topic[i];
 		}
		
 		sub_sock.set_opt(NNG_OPT_SUB_SUBSCRIBE, {topicChArray, lengthWithoutNull});
 		
		sub_sock.dial(streamSockUrl.c_str());	

		LOG_S(INFO) << "Subscribing to messages on " << topic;

		while(true){
			try {
				
				nng::buffer buf = sub_sock.recv(NNG_FLAG_ALLOC);

				std::string_view messageRaw = {buf.data<char>(), buf.size()};
				// std::string messageRaw = buf.data<char>();

				LOG_S(7) << "received: " << buf.size() << " bytes: " << messageRaw ;

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
	int request(json& jrequest, json& response) {
		LOG_S(8) << "Sending request: " << jrequest;		
		try {                    
			auto msg = nng::make_msg(0);
			auto msgStr = jrequest.dump();
			msg.body().append(nng::view(msgStr.c_str(), msgStr.size()));
			req_sock.send(std::move(msg));

			//blocking...
			nng::buffer buf = req_sock.recv();
			std::string_view messageRaw = {buf.data<char>(), buf.size()};
			
			LOG_S(8) << "received response: " << messageRaw << " lenght: " << messageRaw.size();

			if (0 == parseMessage(messageRaw, response)){
				return 0;
			}
		} catch( const nng::exception& e ) {
			LOG_S(WARNING) << "nng Exception: " << e.who() << e.what();			
		}
		return -1;
	}

	int request(const char* raw_request, json& response) {
		json req = json::parse(raw_request);
		return request(req, response);
	}

	void splitMessage( std::string_view msg, std::string& topic, json& jdata) {
		/* we use # as token to separate topics from data */
		std::size_t pos = msg.find("#"); 

		topic = msg.substr(0, pos); 

		std::string_view data{msg.substr(pos+1)}; 
		
		parseMessage(data, jdata);
	}

	int parseMessage( std::string_view msg, json& jrecv) {
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
	std::string reqSockUrl;
	std::string streamSockUrl;

private:
	nng::socket req_sock;
	nng::socket sub_sock;

	std::function<void(ubridge::message&)> _jsonCb;
};