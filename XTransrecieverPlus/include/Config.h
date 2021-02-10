#pragma once
#include <iostream>
#include <fstream>
#include <string>
namespace cfg {

	const std::string searchfilter = "((src or dst portrange 49151-49156) or (src or dst port 30000)) and (udp)";
	const std::string interfaceIPAddr1 = "fe80::78f3:6753:4a95:8b59"; //Your computer local IP addr
	const std::string interfaceIPAddr2 = "fe80::8546:fc1:d36c:b2d8"; //Your computer local IP addr (Same interface as target switch)
	const std::string switchIP = "10.0.0.61";
	const uint32_t selfIPAddrInt = 0x0a0000e0;

};
