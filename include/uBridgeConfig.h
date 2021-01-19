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
	        const char* configSockUrl= "ipc:///tmp/ubridgeConf";
	        // const char* configSockUrl= "tcp://localhost:8001";
	        const char* streamSockUrl= "ipc:///tmp/ubridgeStream";
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
		ping,
		unrecognized,
	};

	struct message {
		std::string topic;
		nlohmann::json data;
	};


}
