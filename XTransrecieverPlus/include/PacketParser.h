#pragma once
#include <vector>
#include <array>
#include <Packet.h>
class Parser {
public:
	enum PacketTypes {
		PIA_MSG = 50,
		BROWSE_REQUEST = 0,
		BROWSE_REPLY = 1
	};

	struct UDPData {
		int srcIP = 0;
		int dstIP = 0;
		int srcPort = 0;
		int dstPort = 0;
		int message_len = 0;
	};

	struct PIAHeader {
		const uint8_t magic[4] = { 0x32, 0xab, 0x98, 0x64 };
		const uint8_t version = 0x84; //PIA Version 5.18
		uint8_t connID = 0;
		uint16_t packetID = 0;
		std::array<uint8_t, 8> nonce;
		std::array<uint8_t, 16> tag;
	} header;

	struct Message {
		//header data
		uint8_t field_flag = 0;
		uint8_t msg_flag = 0;
		uint16_t payload_size = 0;
		uint8_t protocol_type = 0;
		uint8_t protocol_port[3] = { 0,0,0 };
		uint64_t destination = 0;
		uint64_t source_station_id = 0;

		std::vector<uint8_t> payload;
	} message;

	std::vector<uint8_t> raw;

	bool OnPacket(pcpp::Packet packet);
};
