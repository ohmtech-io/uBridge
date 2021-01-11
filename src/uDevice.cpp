#define LOGURU_WITH_STREAMS 1
#include <loguru/loguru.hpp>
// #include <loguru/loguru.cpp>
#include "uSerial.h"
#include "uDevice.h"

namespace ubridge {


const uThingQueries_t uThingQueries;


Uthing::Uthing(const PortName& portName, PortObject portObj): 
					_portName(portName), 
					_port(std::move(portObj)) 
{
	json resp = query(uThingQueries.info);
	if (resp.contains("error")) {
		 throw std::runtime_error("failed to gather device information"); 
	} 
	_devName = resp["info"]["device"];
	_fwVersion = resp["info"]["firmware"];
	_serialNumber = resp["info"]["serial"];
}

auto Uthing::portName() {return _portName;}
auto Uthing::devName() {return _devName;}
auto Uthing::channelID() {return _channelID;}
auto Uthing::fwVersion() {return _fwVersion;}
auto Uthing::serialNumber() {return _serialNumber;}
// upTime() {return lastUpTime + ******}; https://github.com/AnthonyCalandra/modern-cpp-features#stdchrono
auto Uthing::messagesReceived() {return _messagesReceived;}
auto Uthing::messagesSent() {return _messagesSent;}


/* This is static for a device */
json Uthing::info() {
	json info;
	info["info"]["device"] = _devName;
	info["info"]["serial"] = _serialNumber;
	info["info"]["firmware"] = _fwVersion;
	return info;
}

json Uthing::status() {return query(uThingQueries.status);}
	

json Uthing::query(const char* query) {
	try {	
		_port.FlushIOBuffers();	

		LOG_S(6) << "Requesting "<< query;		
		_port.Write(query);
		
		std::string response;
		_port.ReadLine(response, '\n', 1500);
		LOG_S(9) << "Read " << response;

		json jmesg = json::parse(response);
		LOG_S(6) << "JSON resp " << jmesg;	
		return jmesg;
	} catch (const std::exception& ex) {
		LOG_S(WARNING) << _portName << _devName << ": " << ex.what();
		json jerr;
		jerr["error"] = "Failed to get " + std::string(query); 
		return jerr;
	}
}

} //namespace ubridge
