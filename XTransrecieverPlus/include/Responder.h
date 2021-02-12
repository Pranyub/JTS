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

	//this is for proof of concept only. will be changed in the future to actually use a parsed packet.
	bool setPokemonRaw(std::vector<uint8_t> raw);

	bool getResp(std::vector<pcpp::Packet>& out);
	int stage = HANDSHAKE;
	Responder() {};
private:
	Parser *parser;
	int protocol = -1;
	int msgType = -1;
	bool hasResp = false;

	crft::Lan lan;
	crft::Station station;
	void parseLan(std::vector<pcpp::Packet>& packet);
};