#include <iostream>
#include <chrono>

#include "MQ.h"

#define LOGURU_WITH_STREAMS 1
#include <loguru.hpp>
#include <loguru.cpp>

int main(int argc, char *argv[])
{
	MQ *msgQ;

	// loguru::init(argc, argv);
	// Only show most relevant things on stderr:
	loguru::g_stderr_verbosity = 5;


	msgQ = new MQ("configCH", MQ::EndpointType::Client);

	msgQ->sendMessage("/sensor?");

	std::string message;
	int timeout = 3; //seconds
	while(true){
		try 
		{
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
