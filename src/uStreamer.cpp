#include <cstdio>
#include <thread>
#include <functional>

#define LOGURU_WITH_STREAMS 1
#include <loguru/loguru.hpp>

#include <uStreamer.h>

// for convenience
using json = nlohmann::json;

Streamer::Streamer(std::string& url) {

	// create a socket for the PUB protocol
	pub_sock = nng::pub::open();

	streamSockUrl = url;
}

int Streamer::start() {
	try {
		/* PUB socket starts listening */
		pub_sock.listen(streamSockUrl.c_str());	
	} catch( const nng::exception& e ) {
		LOG_S(WARNING) << "nng Exception: " << e.who() << e.what();			
		return -1;
	}	

	LOG_S(INFO) << "Streamer (PUB socket) initialized on " << streamSockUrl;
	return 0;
}

int Streamer::publish(std::string& topic, json& jmessage) {
	
	// LOG_S(5) << "PUB -topic: " << topic << " msg: " << jmessage;
	auto msg = topic + '#' + jmessage.dump();

	LOG_S(7) << "PUB : " << msg;
	pub_sock.send({msg.c_str(), msg.size()});

// 		message.header().clear();
// 		message.body().clear();
// 		message.header().append({topic.c_str(), topic.size()});

// 		std::string serializedMsg = jmessage.dump();
// 		message.body().append({serializedMsg.c_str(), serializedMsg.size()});
// 		pub_sock.send(std::move(message));
	return 0;
}

