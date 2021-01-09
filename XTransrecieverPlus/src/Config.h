#pragma once
#include <iostream>
#include <fstream>
#include <string>
static class Config {
public:
	bool is_live = false; //if false, reads + parses a file. Otherwise, does a live connection

	const std::string searchfilter = "((src or dst portrange 49151-49156) or (src or dst port 30000)) and (udp)";
	const std::string interfaceIPAddr = "10.0.0.224"; //Your computer local IP addr
	const std::string switchIPAddr "10.0.0.61"; //Your switch local IP addr

};