#pragma once

#include <nlohmann/json.hpp>

namespace ubridge {
	class config {
		public:
			std::string appVersionStr = "0.1";
	        std::string devNameBase = "/dev/ttyACM";
	        int maxDevices = 10;
	        const char* configSockUrl= "ipc:///tmp/ubridgeConf";
	        const char* streamSockUrl= "ipc:///tmp/ubridgeStream.ipc";
	};
	/*https://nlohmann.github.io/json/features/arbitrary_types/
	this enable us to convert the class to json (json jcfg = config)
	*/
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(config, appVersionStr, devNameBase, maxDevices)

	enum requestType_t {
		setConfig,
		getConfig,
		getDevices,
		queryDevice,
		ping,
		unrecognized,
	};
}
