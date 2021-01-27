#pragma once

#include <functional>

#include <nlohmann/json.hpp>
#include <nngpp/nngpp.h>

// for convenience
// using json = nlohmann::json;

class ReqRepServer {
public:
	ReqRepServer(std::string& url, std::function<void(nlohmann::json&)> cb);

	int start(); 

	void sendResponse(const std::string rawMsg);

protected:
	void receiveThread();

	int parseMessage(std::string message, nlohmann::json& jrecv);

public: 
	std::string rrSockUrl;

private:
	nng::socket rep_sock;
	std::function<void(std::string&)> _rawCb;
	std::function<void(nlohmann::json&)> _jsonCb;
};