#pragma once

#include <functional>
#include "MQ.h"
#define LOGURU_WITH_STREAMS 1
#include <loguru/loguru.hpp>

using namespace std;

class ReqRepServer {
public:
	ReqRepServer(string name, function<void(string&)> cb) {
		rrMqsQ = new MQ(name, MQ::EndpointType::Server);
		rrMsgQname = name;

		_cb = cb;
	}

	~ReqRepServer() {
		delete rrMqsQ;
	};

	int start() {
		loguru::g_stderr_verbosity = 5;
		LOG_S(INFO) << "Config Server listening on: " << rrMsgQname;

		//capturing the lambda with "this" gives access to all members of the class
		rrMqsQ->listen([this](string message){
			LOG_S(3) << "received raw:" << message;
			// rrMqsQ->sendMessage("Command received!");
			_cb(message);
		});
		return 0;
	};

public: 
	string rrMsgQname;

protected:
private:
	MQ *rrMqsQ;
	function<void(string&)> _cb;
};