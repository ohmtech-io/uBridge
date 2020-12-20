#include <iostream>
#include <chrono>

#include "MQ.h"

using namespace std;

const std::string sampleMessg = "{\"temperature\": 24.68, \"pressure\": 1019.38, \"humidity\": 45.64, \"gasResistance\": 597617, \"IAQ\": 28.3, \"iaqAccuracy\": 3, \"eqCO2\": 511.21, \"eqBreathVOC\": 0.52}";


int main(int argc, char *argv[])
{
	MQ *msgQ;

	bool client1_Subs = false;

	msgQ = new MQ("ubridge", MQ::EndpointType::Server);

	msgQ->listen([msgQ, &client1_Subs](string message){
		cout << "Received: " << message << endl;
		
		if (message == "/sensor?") {
			msgQ->sendMessage("Subscription registered!");
			client1_Subs = true;
		}
	});

	while(true) {
		if (client1_Subs){
			msgQ->sendMessage(sampleMessg);
		}
		this_thread::sleep_for(chrono::microseconds(10));
	}

	delete msgQ;
	return 0;
}
