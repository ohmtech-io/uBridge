#pragma once

#include <nlohmann/json.hpp>

// for convenience
using json = nlohmann::json;

namespace ubridge {
	class config {
		public:
			std::string appVersionStr = "0.1";
	        std::string devNameBase = "/dev/ttyACM";
	        int maxDevices = 10;
	};
	/*https://nlohmann.github.io/json/features/arbitrary_types/*/
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(config, appVersionStr, devNameBase, maxDevices)
}