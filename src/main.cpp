#include <iostream>
#include <chrono>
#include <iomanip>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#define LOGURU_WITH_STREAMS 1
#include <loguru/loguru.hpp>
#include <loguru/loguru.cpp>

#include <nlohmann/json.hpp>

#include "ubridge.h"

using namespace std;


int main(int argc, char *argv[])
{
	loguru::g_preamble_date = false;
	loguru::g_preamble_time = false;

	//TODO: get verbosity from argv...
	loguru::g_stderr_verbosity = 6;
	loguru::init(argc, argv);

	LOG_S(INFO) << "--- Initializing **u-bridge**... ---";


	using namespace ubridge;

	Bridge app;
	app.start();

	int count = 0;


	// #include <libserial/SerialPort.h>
	// try {
	// 	LibSerial::SerialPort port;
	// 	port.Open("/dev/ttyACM0");
	// } catch (const std::exception& ex) {
	// 		LOG_S(WARNING) << ex.what();
	// 		return -1;
	// }

	
	while(true){
		this_thread::sleep_for(chrono::milliseconds(5000));
		// LOG_S(INFO) << "-";
		// app.publish(++count);
		std::string topic = "/sensors/3/data";

		json data;
		data["name"] = "uThingMNL"; 
		data["value"] = ++count; 

		// app.publish(topic, data);
	}
	return 0;
}
