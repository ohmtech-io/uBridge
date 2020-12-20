#include <iostream>
#include <chrono>

#include "MQ.h"

int main(int argc, char *argv[])
{
	MQ *msgQ;

	msgQ = new MQ("ubridge", MQ::EndpointType::Client);

	msgQ->sendMessage("/sensor?");

	while(true){
		std::cout << "Received: " << msgQ->readMessage() << std::endl;
	}

	delete msgQ;
	return 0;
}
