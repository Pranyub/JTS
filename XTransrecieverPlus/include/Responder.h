#pragma once
#include <Packet.h>
#include "PacketParser.h"
#include "PacketCrafter.h"

enum STAGE {
	HANDSHAKE = 1
};

class Responder {
public: 
	void setParser(Parser& parserIn);
	bool getResp(pcpp::Packet& out);
	int stage = HANDSHAKE;
	
private:
	Parser *parser;
	int protocol;
	int msgType;

	crft::Lan lan;
	void parseLan(pcpp::Packet& packet);
};