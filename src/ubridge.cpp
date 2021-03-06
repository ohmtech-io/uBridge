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
#include <filesystem>
#include <regex>
#include <iostream>
#include <thread>
#include <nlohmann/json.hpp>

#define LOGURU_WITH_STREAMS 1
#include <loguru/loguru.hpp>

#include "threadsafeQueue.h"
#include "reqRepServer.h"
#include "uStreamer.h"
#include "uBridgeConfig.h"
#include "uSerial.h"
#include "uDevice.h"
#include "ubridge.h"

// for convenience
using json = nlohmann::json;

using namespace std::chrono_literals;
using namespace std::string_literals;

namespace ubridge {

Bridge::Bridge(Config config): cfg(config) 
{
	/* Calling with object non-static member function requires to bind the implicit "this" pointer.*/
	rrServer = std::make_unique<ReqRepServer>(cfg.configSockUrl, 
					std::bind(&Bridge::requestHandler, 
					this, 
					std::placeholders::_1));

	uStreamer = std::make_unique<Streamer>(cfg.streamSockUrl);
}

void Bridge::start(void) {
	appStartTime = std::chrono::steady_clock::now();

	rrServer->start();
	uStreamer->start();

	inboundQ.setMaxSize(100);

	LOG_S(INFO) << "Starting monitor thread";
	// std::thread monitorPorts(&monitorPortsThread, std::ref(devices), std::ref(mutex_devices), std::ref(cfg));
	
	std::thread monitorPorts(&monitorPortsThread, this);
	monitorPorts.detach();

	json inMessage;
	const std::string base_topic = "/sensors/";

	//Main Bridge loop:
	while (1) {
		inMessage = inboundQ.pop();
		try {
			LOG_S(8) << "Publishing data: " << inMessage;


			std::string topic = base_topic + inMessage["channelID"].get<std::string>();
			inMessage.erase("channelID");

			publish(topic, inMessage);
		}
		catch (const json::exception& e) {
			LOG_S(WARNING) << "Error parsing JSON: " << e.what();
		}
		std::this_thread::sleep_for(5ms);
	}
}

void Bridge::publish(std::string& topic, json& jmessage) {
	uStreamer->publish(topic, jmessage);
}	

int Bridge::listDevices() {

	//this creates an array stored as std::vector
	deviceList["devices"] = {};

	{
		const std::lock_guard<std::mutex> lck(mutex_devices);				

		for (auto& [port, uthing] : devices) {
			json jdevice;
			jdevice["name"] = uthing.devName();
			jdevice["channelID"] = uthing.channelID();
			jdevice["serialNumber"] = uthing.serialNumber();
			jdevice["fwVersion"] = uthing.fwVersion();

			deviceList["devices"].push_back(jdevice);
		}
	}	//unlock	

	deviceList["devCount"] = devices.size();
	
	if (devices.size() == 0) {
		//create an empty array to get consistency so the client doesn't need to
		//check for a null value
		deviceList["devices"] = json::array();	
	}
	LOG_S(5) << "Device list: " << deviceList;
	/*
	{
	   "info": {
	    "device": "uThing::VOC rev.A",
    	"serial": "27D521601B89CE9A",
    	"firmware": "1.2.1"
		}
	}
	*/
	return devices.size();
}

PortName Bridge::findDeviceById(std::string channelID) {
	PortName ret = ""s;
	for (auto& [port, uthing] : devices) {
		if (uthing.channelID() == channelID) {
			ret = port;
			break;
		}
	}
	return ret;
}

json Bridge::getStats() {
	json jstats;

	int count = 0;
	int total_sent_messages = 0;
	int total_recv_messages = 0;

	{
		const std::lock_guard<std::mutex> lck(mutex_devices);

		for (auto& [port, uthing] : devices) {
			const std::string channelID = uthing.channelID();
			jstats["devices"][channelID]["msgSent"] = uthing.messagesSent();
			jstats["devices"][channelID]["msgReceived"] = uthing.messagesReceived();
			uthing.status();
			jstats["devices"][channelID]["upTime"] = uthing.upTime();
			
			total_sent_messages += uthing.messagesSent();
			total_recv_messages += uthing.messagesReceived();
			++count;

		}
	}//unlock

	jstats["numConnectedDevices"] = count;

	auto now = std::chrono::steady_clock::now();
	std::chrono::duration<double, std::milli> elapsed_milis = now - appStartTime;

	// double appUpTime = elapsed_seconds.count()*1000; 
	jstats["bridgeUpTime"] = int(elapsed_milis.count());

	jstats["sentMsgPerSec"] = round(total_sent_messages / elapsed_milis.count() * 100000) / 100; //round to 2 decimal places
	jstats["receivedMsgPerSec"] = round(total_recv_messages / elapsed_milis.count() * 100000) / 100;

	return jstats;
}

json Bridge::sendCommandById(const json& jmessage) {
	if (jmessage.contains("channelID")) {
		PortName portID = findDeviceById(jmessage["channelID"]);
		if (portID.empty()) 
		{
			LOG_S(WARNING) << "In command request - channelID not found:" << jmessage["channelID"];
			return "{\"status\":\"ERROR\",\"error\":\"channelID not found\"}"_json;	
		} else 
		{
			//we have to use the full query since in uThings, all commands reply with at 
			//least a status response, and if we don't catch this response here they will be read by
			//the bridge thread and forwarded as data
			json request = jmessage["command"];
			
			LOG_S(INFO) << "Sending command " << request << portID; 
			{	
				const std::lock_guard<std::mutex> lck(mutex_devices);
				devices.at(portID).jquery(request);
			}
			return "{\"status\":\"OK\"}"_json;
		}
	} else {
		return "{\"status\":\"ERROR\",\"error\":\"channelID required\"}"_json;
	}
}

json Bridge::queryDeviceById(const json& jmessage) {
	if (jmessage.contains("channelID")) {
		PortName portID = findDeviceById(jmessage["channelID"]);
		if (portID.empty()) 
		{
			LOG_S(WARNING) << "In query request - channelID not found:" << jmessage["channelID"];
			return "{\"status\":\"ERROR\",\"error\":\"channelID not found\"}"_json;	
		} else 
		{
			json response;
			json request = jmessage["query"];
			
			LOG_S(INFO) << "Querying device " << request << portID; 
			{	
				const std::lock_guard<std::mutex> lck(mutex_devices);
				response = devices.at(portID).jquery(request);
			}
			LOG_S(INFO) << response;

			return response;
		}
	} else {
		return "{\"status\":\"ERROR\",\"error\":\"channelID required\"}"_json;
	}
}

void Bridge::prepareResponse(requestType_t& requestType, const json& jmessage) {
	std::string response = "{\"status\":\"unknown error\"}";
	json jcfg = cfg;

	LOG_S(9) << "request type ID" << requestType;

	switch(requestType) {
		case ping:
			response = "{\"pong\":1}";
			break;
		case getConfig:
			/* send the actual Bridge configuration converted to JSON*/
			response = jcfg.dump();
			break;
		case setConfig:
			LOG_S(WARNING) << "TODO: implement this!!!";
			response = "{\"status\":\"ERROR\",\"error\":\"feature not implemented\"}";
			break;
		case getDevices:
			listDevices();
			response = deviceList.dump();
			break;
		case queryDevice:
			response = queryDeviceById(jmessage).dump();
			break;
		case sendCommand:
			response = sendCommandById(jmessage).dump();				
			break;
		case getStatistics:
			response = getStats().dump();				
			break;
		case unrecognized:
			response = "{\"status\":\"ERROR\",\"error\":\"command not valid\"}";
			break;
	}

	rrServer->sendResponse(response);
}

int Bridge::parseRequest(const json& jmessage) {
	int ret = -1;
	
	if (jmessage["request"] == "setConfig") {
		/*we can't use the arbitrary_types feature to fill the config here, if one key is missing, or doesn't match, it fails*/
		if (jmessage.contains("maxDevices")) {
			//nngcat --req --dial ipc:///tmp/ubridgeReqResp --data "{\"request\":\"setConfig\",\"maxDevices\":1}";		
			requestType = setConfig;	
			cfg.maxDevices = jmessage["maxDevices"];
			ret = 0;
		} 
		if (jmessage.contains("devNameBase")) {
			requestType = setConfig;
			cfg.devNameBase = jmessage["devNameBase"];
			ret = 0;
		}
		/* just checking the results..*/
		json j = cfg;
		LOG_S(3) << "actual config: " << j;
	} 
	if (jmessage["request"] == "getConfig") {
		// nngcat --req --dial ipc:///tmp/ubridgeReqResp --data "{\"request\":\"getConfig\"}";
		requestType = getConfig;	
		ret = 0;
	}
	if (jmessage["request"] == "getDevices") {
		// nngcat --req --dial ipc:///tmp/ubridgeReqResp --data "{\"request\":\"getDevices\"}";
		requestType = getDevices;	
		ret = 0;
	}
	if (jmessage["request"] == "queryDevice") {
		// nngcat --req --dial ipc:///tmp/ubridgeReqResp --data "{\"request\":\"queryDevice\", \"channelID\":\"uThing::MNL_C4C5\", \"query\":{\"status\":true}}";
		requestType = queryDevice;	
		ret = 0;
	}		
	if (jmessage["request"] == "sendCommand") {
		// nngcat --req --dial ipc:///tmp/ubridgeReqResp --data "{\"request\":\"sendCommand\", \"channelID\":\"uThing::MNL_C4C5\", \"command\":{\"led\":true}}";
		requestType = sendCommand;	
		ret = 0;
	}	
	if (jmessage["request"] == "getStatistics") { 
		// nngcat --req --dial ipc:///tmp/ubridgeReqResp --data "{\"request\":\"getStatistics\"}";
		requestType = getStatistics;	
		ret = 0;
	}	
	return ret;
}

void Bridge::requestHandler(json& jmessage)	{
	LOG_S(6) << "Request_Handler> JSON message: " << jmessage;

	requestType = unrecognized;

	if (jmessage.contains("ping")) {
		requestType = ping;
	} else if (jmessage.contains("request")) {
		if (parseRequest(jmessage) != 0) {
			LOG_S(WARNING) << "Error parsing command!";
		}
	}
	prepareResponse(requestType, jmessage);
}

} //namespace ubridge
