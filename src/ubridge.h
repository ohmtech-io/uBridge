#pragma once

#include <filesystem>
#include <regex>
#include <nlohmann/json.hpp>

#include "threadsafeQueue.h"
#include "reqRepServer.h"
#include "uStreamer.h"
#include "uBridgeConfig.h"
#include "uSerial.h"
#include "uDevice.h"


// for convenience
using json = nlohmann::json;

namespace ubridge {

class Bridge {
public:
	Bridge(Config config): cfg(config) 
	{
		/* Calling with object non-static member function requires to bind the implicit "this" pointer.*/
		rrServer = std::make_unique<ReqRepServer>(cfg.configSockUrl, 
												std::bind(&Bridge::requestHandler, 
												this, 
												std::placeholders::_1));

		uStreamer = std::make_unique<Streamer>(cfg.streamSockUrl);
	}

	~Bridge(){
	};

	void start(void) {
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

private:
	void publish(std::string& topic, json& jmessage) {
		uStreamer->publish(topic, jmessage);
	}	

	int listDevices() {

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

	PortName findDeviceById(std::string channelID) {
		PortName ret = ""s;
		for (auto& [port, uthing] : devices) {
			if (uthing.channelID() == channelID) {
				ret = port;
				break;
			}
		}
		return ret;
	}

	json getStats() {
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

	json sendCommandById(const json& jmessage) {
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
					//TODO: shouldn't we use a mutex on the specific instance instead?
					//i.e. each instance will have their own mutex too
					devices.at(portID).jquery(request);
				}
				return "{\"status\":\"OK\"}"_json;
			}
		} else {
			return "{\"status\":\"ERROR\",\"error\":\"channelID required\"}"_json;
		}
	}

	json queryDeviceById(const json& jmessage) {
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
					//TODO: shouldn't we use a mutex on the specific instance instead?
					//i.e. each instance will have their own mutex too
					response = devices.at(portID).jquery(request);
				}
				LOG_S(INFO) << response;

				return response;
			}
		} else {
			return "{\"status\":\"ERROR\",\"error\":\"channelID required\"}"_json;
		}
	}

	void prepareResponse(requestType_t& requestType, const json& jmessage) {
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

	int parseRequest(const json& jmessage) {
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
		// todo: do this nicely man
		return ret;
	}

	void requestHandler(json& jmessage)	{
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

public:
	Config cfg;	
	json deviceList;
	std::map<PortName, Uthing> devices;
	std::mutex mutex_devices;

	TQueue<json> inboundQ;
	TQueue<json> outboundQ;

private:
	std::unique_ptr<ReqRepServer> rrServer;
	std::unique_ptr<Streamer> uStreamer;
	requestType_t requestType;
	PortList portsNameList;

	std::chrono::time_point<std::chrono::steady_clock> appStartTime; 
}; //class Bridge 
} //namespace ubridge