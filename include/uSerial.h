#pragma once

#include <mutex>
#include <libserial/SerialPort.h>
#include "uBridgeConfig.h"
#include "uDevice.h"
// #include "../src/ubridge.h"

namespace ubridge {

using PortName = std::string;
using PortObject = LibSerial::SerialPort;
using PortList = std::vector<PortName>;	
using Devices = std::map<PortName, Uthing>;

class Bridge;
// void monitorPortsThread(Devices& devices, std::mutex& mutex_devices, Config& config);
void monitorPortsThread(Bridge* bridge);
void findPorts(std::string devNameBase, PortList& portList);
bool isUthing(const PortName& fileDescriptor, PortObject& port);
	

}//namespace ubridge