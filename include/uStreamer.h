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
		return 0;
	};

	int publish(std::string& topic, json& jmessage) {
		
		// LOG_S(5) << "PUB -topic: " << topic << " msg: " << jmessage;
		auto msg = topic + '#' + jmessage.dump();

		LOG_S(5) << "PUB : " << msg;
		pub_sock.send({msg.c_str(), msg.size()});

	// 		message.header().clear();
	// 		message.body().clear();
	// 		message.header().append({topic.c_str(), topic.size()});

	// 		std::string serializedMsg = jmessage.dump();
	// 		message.body().append({serializedMsg.c_str(), serializedMsg.size()});
	// 		pub_sock.send(std::move(message));
		return 0;
	}

public: 
	const char* streamSockUrl;

private:
	nng::socket pub_sock;
	nng::msg message = nng::make_msg(0);
};
