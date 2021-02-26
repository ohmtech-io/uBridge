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

