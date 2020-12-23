#include <iostream>
#include <chrono>
#include <iomanip>

#define LOGURU_WITH_STREAMS 1
#include <loguru/loguru.hpp>
#include <loguru/loguru.cpp>

#include <nlohmann/json.hpp>

#include "MQ.h"
#include "ubridge.h"

// for convenience
using json = nlohmann::json;

ubridge::config cfg;

int main(int argc, char *argv[])
{
	loguru::g_stderr_verbosity = 5;
	loguru::g_preamble_date = false;
	loguru::g_preamble_time = false;
	// loguru::init(argc, argv);

	MQ *msgQ;
	msgQ = new MQ(cfg.configChName, MQ::EndpointType::Client);

	LOG_S(INFO) << "Start client - Config ch: " << cfg.configChName;

	/* wait for ubridge config server */
	{
		int timeout = 1; //seconds
		std::string message;
		bool serverOnline = false;
		do {
				msgQ->sendMessage("{\"ping\":1}");
				LOG_S(5) << ">> PING >>";

			try 
			{
				/* blocking*/
				message =  msgQ->readMessage(timeout);

				// json jmessage;
				// if (0 == parseMessage(message, jmessage)){
				// 	serverOnline = true;
				// }
				serverOnline = true;
				// std::cout << "Received: " << message << std::endl;
				LOG_S(5) << "Received: " << message;
			} 
			catch (MQ::ErrorType error)
			{
				if (error == 0){
				// if (error == MQ::ErrorType::Timeout){
					LOG_S(WARNING) << "Message read timeout";	
				} else {
					LOG_S(WARNING) << "Error reading msg - ErrorType: " << error;
				}	
			}		
		} while (!serverOnline);
	}
	// ubridge::config cfg;
	// cfg.maxDevices = 2;
	// json jsoncfg = cfg;
	// msgQ->sendMessage(jsoncfg.dump());

    json jsoncfg;
    // jsoncfg["maxDevices"] = 3;
    //or
    jsoncfg = {{"maxDevices", 3}};
    // jsoncfg = {{"maxDevices", 3}, {"devNameBase", "/dev/andaadormir"}};

    /* serialize and send */
	msgQ->sendMessage(jsoncfg.dump());
	LOG_S(5) << "SEND: " << jsoncfg;

	std::string message;
	int timeout = 3; //seconds

	while(true){
		try 
		{
			/* blocking*/
			message =  msgQ->readMessage(timeout);
			// std::cout << "Received: " << message << std::endl;
			LOG_S(5) << "Received: " << message;
		} 
		catch (MQ::ErrorType error)
		{
			LOG_S(WARNING) << "Error reading msg - ErrorType: " << error;
			// std::cout << "Error reading msg - ErrorType: " << error << std::endl;	
		}		
	}

	delete msgQ;
	return 0;
}
