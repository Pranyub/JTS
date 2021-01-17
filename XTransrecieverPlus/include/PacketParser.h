#pragma once
#include <vector>
#include <array>
#include <Packet.h>

enum PROTOCOL {
	LAN = 0x44
};

enum PacketTypes {
	PIA_MSG = 0x32,
	BROWSE_REQUEST = 0,
	BROWSE_REPLY = 1
};

class Parser {
public:

	Parser() {
		raw = new std::vector<uint8_t>;
	}

	Parser(const Parser& parserOld) {
		//raw.clear();
		message.payload.clear();
	}

	int test = 4;

	struct UDPData {
		int srcIP = 0;
		int dstIP = 0;
		int srcPort = 0;
		int dstPort = 0;
		int message_len = 0;
	} udpInfo;

	struct PIAHeader {
		uint8_t magic[4] = { 0x32, 0xab, 0x98, 0x64 };
		uint8_t version = 0x84; //PIA Version 5.18
		uint8_t connID = 0;
		uint16_t packetID = 0;
		std::array<uint8_t, 8> nonceCounter;
		std::array<uint8_t, 16> tag;

		//takes an iterator pointing at the beginning of PIA Header and sets values
		//passing an iter by reference is a bit wonky so its done by value instead
		std::vector<uint8_t>::iterator fill(std::vector<uint8_t>::iterator iter);

	} header;

	struct Message {
		//header data
		uint8_t field_flags = 0;
		uint8_t msg_flag = 0;
		uint16_t payload_size = 0;
		uint8_t protocol_type = 0;
		uint8_t protocol_port[3] = { 0,0,0 };
		uint64_t destination = 0;
		uint64_t source_station_id = 0;

		std::vector<uint8_t> payload;
		//parses a PIA Message from a raw input
		int setMessage(std::vector<uint8_t> data);
	} message;

	const uint8_t GAME_KEY[16] = { 112, 49, 102, 114, 88, 113, 120, 109, 101, 67, 90, 87, 70, 118, 48, 88 }; //Game specific key used for encryption
	

	//Persistent variables
	bool decryptable = false; //can't decrypt unless session key is set via setSessionKey()
	std::array<uint8_t, 4> sessionID;
	std::array<uint8_t, 16> sessionKey; //key used for decryption

	bool onPacket(pcpp::Packet packet);

	void resetAll();
private:

	std::vector<uint8_t>* raw;
	bool parsePia(std::vector<uint8_t> piaMsg);
	
	//decrypts a given packet with sessionKey
	bool DecryptPia(const std::vector<uint8_t> encrypted, std::vector<uint8_t>* decrypted);
	
	bool parseBrowseRequest();
	bool parseBrowseReply();
	
	void setSessionKey(const uint8_t mod_param[]);
	//empty structs that can be used for resetting
	const UDPData udpInfoReset;
	const PIAHeader headerReset;
	const Message messageReset;
};
