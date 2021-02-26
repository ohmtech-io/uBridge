#pragma once

#include <nlohmann/json.hpp>
#include "threadsafeQueue.h"
#include "reqRepServer.h"
#include "uStreamer.h"
#include "uBridgeConfig.h"
#include "uSerial.h"
#include "uDevice.h"

namespace ubridge {

class Bridge {
public:
	Bridge(Config config);
	void start(void);

private:
	void publish(std::string& topic, nlohmann::json& jmessage);
	int listDevices();
	PortName findDeviceById(std::string channelID);
	nlohmann::json getStats();
	nlohmann::json sendCommandById(const nlohmann::json& jmessage);
	nlohmann::json queryDeviceById(const nlohmann::json& jmessage);

	void prepareResponse(requestType_t& requestType, const nlohmann::json& jmessage);

	int parseRequest(const nlohmann::json& jmessage);
	void requestHandler(nlohmann::json& jmessage);

public:
	Config cfg;	
	nlohmann::json deviceList;
	std::map<PortName, Uthing> devices;
	std::mutex mutex_devices;

	TQueue<nlohmann::json> inboundQ;
	TQueue<nlohmann::json> outboundQ;

private:
	std::unique_ptr<ReqRepServer> rrServer;
	std::unique_ptr<Streamer> uStreamer;
	requestType_t requestType;
	PortList portsNameList;

	std::chrono::time_point<std::chrono::steady_clock> appStartTime; 
}; //class Bridge 
} //namespace ubridge