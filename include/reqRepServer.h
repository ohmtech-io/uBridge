#pragma once

#include <functional>

#define LOGURU_WITH_STREAMS 1
#include <loguru/loguru.hpp>

#include <nlohmann/json.hpp>
#include "MQ.h"

using namespace std;
// for convenience
using json = nlohmann::json;


class ReqRepServer {
public:
	ReqRepServer(string name, function<void(json&)> cb) {
		rrMqsQ = new MQ(name, MQ::EndpointType::Server);
		rrMsgQname = name;

		_jsonCb = cb;
	}

	~ReqRepServer() {
		delete rrMqsQ;
	};

	int start() {
		loguru::g_stderr_verbosity = 5;
		LOG_S(INFO) << "Config Server listening on: " << rrMsgQname;

		/* Register a lambda expr. as async receiver.
			capturing the lambda with "this" gives access to all members of the class*/
		rrMqsQ->listen([this](string message){
			LOG_S(3) << "received raw:" << message;
			// rrMqsQ->sendMessage("Command received!");

			json jmessage;
			if (0 == parseMessage(message, jmessage)){
				_jsonCb(jmessage);	
			}
		});
		return 0;
	};

	void sendResponse(std::string rawMsg) {
		rrMqsQ->sendMessage(rawMsg);
	}

protected:
	int parseMessage(string& message, json& jrecv){
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
	string rrMsgQname;

private:
	MQ *rrMqsQ;
	function<void(string&)> _rawCb;
	function<void(json&)> _jsonCb;
};