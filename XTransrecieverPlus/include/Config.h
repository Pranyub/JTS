#pragma once
#include <iostream>
#include <fstream>
#include <string>
namespace cfg {

	const std::string searchfilter = "((src or dst portrange 49151-49156) or (src or dst port 30000)) and (udp)";
	const std::string interfaceIPAddr2 = "10.13.0.224"; //Your computer local IP addr
	const std::string interfaceIPAddr1 = "fe80::a459:899:8252:d0ca"; //Your computer local IP addr
	const std::string switchIP = "10.13.0.224";
	const uint32_t selfIPAddrInt = 0x0a0000e0;

};
