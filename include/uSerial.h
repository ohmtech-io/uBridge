#pragma once

#include <mutex>
#include <libserial/SerialPort.h>
#include "uBridgeConfig.h"
#include "uDevice.h"

namespace ubridge {

using PortName = std::string;
using PortObject = LibSerial::SerialPort;
using PortList = std::vector<PortName>;	
using Devices = std::map<PortName, Uthing>;

void monitorPortsThread(Devices& devices, std::mutex& mutex_devices, Config& config);
void findPorts(std::string devNameBase, PortList& portList);
bool isUthing(const PortName& fileDescriptor, PortObject& port);
	

}//namespace ubridge