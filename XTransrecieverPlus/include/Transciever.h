#pragma once
#include "PacketParser.h"
class Tx {
public:
	struct Cookie {
		Parser parser;
	};

	pcpp::PcapLiveDevice* dev;
	void Start(const std::string interfaceIPAddr, const std::string switchIPAddr, const std::string searchfilter);
};