#pragma once
#include <Packet.h>
#include "PacketParser.h"
#include "PacketCrafter.h"

enum STAGE {
	HANDSHAKE = 1
};

enum PROTOCOL {
	LAN = 0x44
};

class Responder {
public: 
	void setParser(Parser* parserIn);
	bool getResp(pcpp::Packet *out);
	int stage = HANDSHAKE;
	
private:
	Parser *parser;
	pcpp::Packet *packet;
	int protocol;
	int msgType;

	crft::Lan lan;
	void parseLan();
};