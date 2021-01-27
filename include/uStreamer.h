#pragma once

#include <nlohmann/json.hpp>
#include <nngpp/nngpp.h>
#include <nngpp/protocol/pub0.h>

class Streamer {
public:
	Streamer(std::string& url);
	
	int start(); 
	int publish(std::string& topic, nlohmann::json& jmessage);

public: 
	std::string streamSockUrl;

private:
	nng::socket pub_sock;
	nng::msg message = nng::make_msg(0);
};
