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

#include <chrono>
#include <nlohmann/json.hpp>

#include "threadsafeQueue.h"
#include "uSerial.h"

namespace ubridge {
using json = nlohmann::json;

using PortName = std::string;
using PortObject = LibSerial::SerialPort;
using PortList = std::vector<PortName>;	

class Bridge;

struct uThingQueries_t {
	const char* info = "{\"info\": true}\n";
	const char* status = "{\"status\": true}\n";
	const json j_info = json::parse(info);
	const json j_status = json::parse(status);
};

class Uthing {
public:	
	Uthing(const PortName& portName, PortObject portObj);

	//getters:
	std::string portName();
	std::string devName(); 
	std::string channelID();
	std::string fwVersion();
	std::string serialNumber();
	int upTime(); //{return lastUpTime + ******}; https://github.com/AnthonyCalandra/modern-cpp-features#stdchrono
	int messagesReceived();
	int messagesSent();
	json info(); /* This is static for a device */
	json status();

	// void setChannelID(const std::string& channelID);
	void assignChannelID();

	void relayThread(Bridge& bridge);
	// void relayThread(TQueue<json>& inboundQueue, TQueue<json>& outboundQueue);
	
	json query(const char* raw_query);
	json jquery(const json& query);

private:
	//Port:
	PortName _portName;
	PortObject _port;

	//Device:
	std::string _devName;	//exampole: "uThing::VOC rev.A", multiple devices with the same name possible
	std::string _fwVersion;
	std::string _serialNumber;
	//used for Pub/Sub
	std::string _channelID; //unique

	//some stats:
	// std::chrono::milliseconds _lastUpTime;
	int _lastUpTime = 0;

	int _messagesReceived = 0;
	int _messagesSent = 0;		
}; //class 
} //namespace ubridge