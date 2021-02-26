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