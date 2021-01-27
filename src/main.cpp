#include <iostream>
#include <fstream>
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

#include <cxxopts.hpp>

#include "ubridge.h"

using namespace std;


int main(int argc, char *argv[])
{
	cxxopts::Options options("ubridge", "Manages connected uThing devices and acts as a broker for the sensor data and configuration");

    options.add_options()
        // ("c,config", "JSON configuration file name", cxxopts::value<std::string>()->default_value("ubridgeConfig.json"))
    ("c,config", "JSON configuration file name", cxxopts::value<std::string>())
        ("v,verbose", "Verbosity output level (0-9)", cxxopts::value<int>()->default_value("0"))
        ("h,help", "Print usage")
    ;

    std::string config_file = "ubridgeConfig.json";

    try {
    	auto result = options.parse(argc, argv);

    	if (result.count("help")) {
      		std::cout << options.help() << std::endl;
      		exit(0);
    	}

    	if (result.count("verbose")) {
    		loguru::g_stderr_verbosity = result["verbose"].as<int>();
    	}

    	if (result.count("config")) {
      		config_file = result["config"].as<std::string>();
    	}
    }
    catch (const cxxopts::OptionException& e) {
    	std::cout << "error parsing options: " << e.what() << std::endl;
    	exit(1);
  	}

	loguru::g_preamble_date = false;
	loguru::g_preamble_time = false;

	//TODO: get verbosity from argv...
	// loguru::g_stderr_verbosity = 6;

	loguru::init(argc, argv);

	// Only log INFO, WARNING, ERROR and FATAL
	loguru::add_file("/tmp/ubridge.log", loguru::Truncate, loguru::Verbosity_INFO);

	json jconfig; //create unitiialized json object

	try {

		LOG_S(INFO) << "Opening " << config_file;
		// read in the json file
		std::ifstream file(config_file, std::ifstream::in);

		if (not file.is_open()) {
			LOG_S(ERROR) << "Error, couldn't open configuration file " << config_file;
			exit(1);
		}

   		file >> jconfig; // initialize json object with what was read from file

    	std::cout << jconfig << std::endl; // prints json object to screen

    	// uses at to access fields from json object
    	// std::cout << jconfig.at("testField") << std::endl;
	} catch (const std::runtime_error& ex) {
		LOG_S(WARNING) << "Error parsing options: " << ex.what();
	}


	LOG_S(INFO) << "--- Initializing ** u-bridge **... ---";

	// using namespace ubridge;


	ubridge::Config config;

	//override defaults on uBridgeConfig.h if they are present on the json file
	if (jconfig.contains("devNameBase")) {
		config.devNameBase = jconfig.at("devNameBase");
	}

	if (jconfig.contains("configSockUrl")) {
		config.configSockUrl = jconfig.at("configSockUrl");
	}

	if (jconfig.contains("streamSockUrl")) {
		config.streamSockUrl = jconfig.at("streamSockUrl");
	}

	ubridge::Bridge app(config);

	app.start();

	// int count = 0;
	
	// while(true){
	// 	this_thread::sleep_for(chrono::milliseconds(10000));
	// 	// LOG_S(INFO) << "-";
	// 	// app.publish(++count);
	// 	std::string topic = "/sensors/3/data";

	// 	json data;
	// 	data["name"] = "uThingMNL"; 
	// 	data["value"] = ++count; 

	// 	// app.publish(topic, data);
	// 	// delete app;
	// 	// return(0);
	// }
	return 0;
}
