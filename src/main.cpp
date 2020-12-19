#include <iostream>
#include <chrono>

#include "MQ.h"

int main(int argc, char *argv[])
{
	MQ *msgQ;

	msgQ = new MQ("hello", MQ::EndpointType::Server);
	msgQ->listen([msgQ](std::string message){
		std::cout << "Received: " << message << std::endl;
		msgQ->sendMessage("Hii!");
	});
	while(true) std::this_thread::sleep_for(std::chrono::seconds(1));

	std::string fact = "Hello worldd";
	std::cout << fact << std::endl;

	delete msgQ;
	return 0;
}
