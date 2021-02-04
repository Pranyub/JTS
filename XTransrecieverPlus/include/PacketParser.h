#pragma once
#include <vector>
#include <array>
#include <Packet.h>
#include "Util.h"
enum PROTOCOL {
	LAN = 0x44,
	STATION = 0x14
};

enum PacketTypes {
	PIA_MSG = 0x32,
	BROWSE_REQUEST = 0,
	BROWSE_REPLY = 1
};

//Game-Specific key used for packet encryption
const uint8_t GAME_KEY[16] = { 0x70, 0x31, 0x66, 0x72, 0x58, 0x71, 0x78, 0x6d, 0x65, 0x43, 0x5a, 0x57, 0x46, 0x76, 0x30, 0x58 };

class Parser {
public:

	Parser() {
		raw = new std::vector<uint8_t>;
	}

	Parser(const Parser& parserOld) {
		//raw.clear();
		recv_message.payload.clear();
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
		//nonceCounter in integer form
		uint64_t nonce = 1;
		std::vector<uint8_t> headerNonce; //only used in packet decryption
		std::array<uint8_t, 16> tag;
		
		//takes an iterator pointing at the beginning of PIA Header and sets values
		//passing an iter by reference is a bit wonky so its done by value instead
		std::vector<uint8_t>::iterator fill(std::vector<uint8_t>::iterator iter);
		std::vector<uint8_t> set();
	} recv_header;

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
		int setMessage(std::vector<uint8_t> data, int offset=0);

		//returns a Message vector from values
		std::vector<uint8_t> getMessage();

		//appends header to given vector
		void appendHeader(std::vector<uint8_t>* data);
	} recv_message;
	
	std::vector<Message> messageVector;

	struct CryptoChallenge {
		uint8_t version = 2;
		uint8_t enabled = 1;

		//nonce in integer form
		uint64_t* nonce;
		std::array<uint8_t, 16> challengeKey; //browse request key
		std::vector<uint8_t> selfKey; //browse response key
		std::array<uint8_t, 16> challengeTag;
		std::vector<uint8_t> challenge;

		CryptoChallenge() {
			util::HexToVector("98408530300f066d20cf8fafa062cd87", &selfKey);
		}

		CryptoChallenge(uint64_t* setNonce) {
			nonce = setNonce;
			util::HexToVector("98408530300f066d20cf8fafa062cd87", &selfKey);
		}

		bool parseChallenge(std::vector<uint8_t> raw, std::array<uint8_t, 12>* challengeNonce);
		std::vector<uint8_t> makeChallenge();
		//uses the DECRYPTED challenge and returns the full challenge response. Session param not included.
		std::vector<uint8_t> makeResponse();

	} browseReply{&recv_header.nonce};

	//const uint8_t GAME_KEY[16] = { 112, 49, 102, 114, 88, 113, 120, 109, 101, 67, 90, 87, 70, 118, 48, 88 }; //Game specific key used for encryption
	std::vector<uint8_t>* raw;

	//Persistent variables
	bool decryptable = false; //can't decrypt unless session key is set via setSessionKey()
	std::array<uint8_t, 4> sessionID;
	std::array<uint8_t, 16> sessionKey; //key used for decryption

	//Start parsing a given packet - creates a PiaHeader & PiaMessages
	bool onPacket(pcpp::Packet packet);

	//encrypts a packet with sessionKey and applies a given PIAHeader to the output
	bool EncryptPia(std::vector<uint8_t> decrypted, std::vector<uint8_t>* encrypted, PIAHeader header_self);

	void resetAll();
private:

	//parses a PIA Packet into a PIA Header and PIA Messages
	bool parsePia(std::vector<uint8_t> piaMsg);
	
	//decrypts a given packet with sessionKey. Returns decrypted messages (raw data)
	bool DecryptPia(const std::vector<uint8_t> encrypted, std::vector<uint8_t>* decrypted);

	bool parseBrowseRequest();
	bool parseBrowseReply();
	
	void setSessionKey(const uint8_t mod_param[]);
	//empty structs that can be used for resetting
	const UDPData udpInfoReset;
	const PIAHeader headerReset;
	const Message messageReset;
};
