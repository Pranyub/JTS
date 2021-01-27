#pragma once
#include "PacketParser.h"
#include "Responder.h"
#include <PcapLiveDeviceList.h>
class Tx {
public:
	struct Cookie {
		Parser parser;
		Responder responder;
		Cookie() {};
	};
	
	pcpp::PcapLiveDevice* dev;
	void Start(const std::string interfaceIPAddr, const std::string switchIPAddr, const std::string searchfilter);
};