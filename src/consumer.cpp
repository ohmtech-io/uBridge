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

#include "uBridgeConfig.h"
#include "ubridgeClient.h"

// for convenience
using json = nlohmann::json;

using namespace std;

ubridge::Config cfg;

void subsMessageHandler(ubridge::message& message) {
	LOG_S(9) << "cb msg" << message.topic << message.data ;
}

int main(int argc, char *argv[])
{
	
	int verb_level = 0;

	if (argc > 1) { 
		verb_level = atoi(argv[1]);
	}
	loguru::g_stderr_verbosity = verb_level;
	
	loguru::g_preamble_date = false;
	loguru::g_preamble_time = false;
	// loguru::init(argc, argv);

	ReqRepClient client(cfg.configSockUrl, cfg.streamSockUrl.c_str());


	LOG_S(INFO) << "*** Starting client... ***" ;
	
	LOG_S(INFO) << "Pinging server..." ;
	if (client.connect() != 0) {
		/* wait for ubridge config server */
		LOG_S(INFO) << "Server not found!" ;	
		LOG_S(INFO) << "Waiting for server..." ;	
		while (client.connect() !=0) {
			this_thread::sleep_for(chrono::milliseconds(1000));
		}
	}
		
	json deviceList;

	client.getDevices(deviceList);

	LOG_S(INFO) << deviceList["devCount"] << " devices detected. Details:" << std::setw(2) << deviceList["devices"];	
	// ubridge::config cfg;
	// cfg.maxDevices = 2;
	// json jsoncfg = cfg;
	
	json query = "{\"status\":true}"_json;
	json command = "{\"led\":true}"_json;
	// json query = "{\"led\":false}"_json;
	json resp;

	std::string chID = "uThing::VOC_9142";
	client.queryDeviceById(chID, query, resp);

	client.sendCommand(chID, command, resp);

	client.getStatistics(resp);
	LOG_S(INFO) << "Statistics:" << std::setw(2) << resp;	

	//start message receiving loop...
	client.subscribe("/sensors", subsMessageHandler); //subscribe to all sensors
	// client.subscribe("/sensors/uThing::VOC_9142", subsMessageHandler); //specific one

    // json jsoncfg;
    // jsoncfg["maxDevices"] = 3;
    //or
    // jsoncfg = {{"maxDevices", 3}};
    // jsoncfg = {{"maxDevices", 3}, {"devNameBase", "/dev/andaadormir"}};

	return 0;
}
