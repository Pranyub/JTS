#pragma once
#include "PacketParser.h"
#include "Responder.h"
#include <PcapLiveDeviceList.h>





class Tx {
public:
	struct Cookie {
		Parser parser;
		Responder responder;
		pcpp::PcapLiveDevice* output;
		pcpp::MacAddress selfSwitchMac{"SOMETHING INVALID"};
		pcpp::MacAddress* otherSwitchMac;

		Pokemon selfPokemon;
		Pokemon* destPokemon;
		Pokemon* injectPokemon;

		bool isSecondary;

		void getPacket(pcpp::Packet& in) {
			isReady = false;
			in = packet;
			return;
		}
		pcpp::Packet packet;
		bool isReady = false;
		Cookie() {};
		
	} cookie;
	
	

	pcpp::PcapLiveDevice* dev;
	
	Tx(const std::string interfaceIPAddr, const std::string switchIPAddr, const std::string searchfilter, const bool secondary=false);
	
	void Start();

};