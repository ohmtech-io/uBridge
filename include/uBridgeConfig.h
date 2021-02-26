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

#include <nlohmann/json.hpp>

namespace ubridge {
	class Config {
		public:
			std::string appVersionStr = "0.1";

			/* NOTE: define a custom path base for the USB devices (if a udev rule is installed for this
			reason, or the kernel driver assigns anything different than ttyACM* or ttyUSB*)
			*/
	        // std::string devNameBase = "/dev/ttyUTHING";
	        std::string devNameBase = "/dev/ttyACM";
	        // std::string devNameBase = ""; //leaving an empty string looks for /dev/ttyACM* and /dev/ttyUSB*

	        int maxDevices = 10;
	        // const char* configSockUrl= "ipc:///tmp/ubridgeReqResp";
	        std::string configSockUrl= "ipc:///tmp/ubridgeReqResp";
	        // const char* configSockUrl= "tcp://localhost:8001";
	        std::string streamSockUrl= "ipc:///tmp/ubridgeStream";
	        // const char* streamSockUrl= "tcp://localhost:8000";
	};
	/*https://nlohmann.github.io/json/features/arbitrary_types/
	this enable us to convert the class to json (json jcfg = config)
	*/
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Config, appVersionStr, devNameBase, maxDevices)

	enum requestType_t {
		setConfig,
		getConfig,
		getDevices,
		queryDevice,
		sendCommand,
		getStatistics,
		ping,
		unrecognized,
	};

	struct message {
		std::string topic;
		nlohmann::json data;
	};


}
