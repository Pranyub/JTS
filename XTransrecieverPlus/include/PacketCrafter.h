#include "PacketParser.h"
#include <Packet.h>
#include <EthLayer.h>
#include <IPv4Layer.h>
#include <UdpLayer.h>
#include <PayloadLayer.h>
#include "Util.h"
#include "Config.h"
namespace crft {

	struct PiaPacket {
		int protocolType = 0;
		int messageType = 0;
		
		//nonce increment counter
		static uint64_t nonce;

		Parser* parser = nullptr;
		Parser::PIAHeader header;
		Parser::Message message;
		PiaPacket(Parser* parserIn);
		PiaPacket();
	protected:
		pcpp::Packet craftPacket(std::vector<uint8_t> data, int srcPort, int dstPort, int dstIP = 0x0a0000ff /*broadcast by default*/);
	};

	struct Lan : public PiaPacket {
		enum MSG_TYPES {
			BROWSE_REQ = 0,
			BROWSE_REP = 1,
			GET_HOST_REQ = 3,
			GET_HOST_REP = 4,
			GET_SESSION_REQ = 5,
			GET_SESSION_REP = 6,
			KEEP_ALIVE = 7
		};

		bool shouldRep = true;

		Lan(Parser *parserIn) : PiaPacket(parserIn) {
			message.protocol_type = LAN;
			

		}
		Lan() {}
		void craftBrowseReq(pcpp::Packet& in);
		void craftBrowseRep(pcpp::Packet& in);
		void craftHostReq(pcpp::Packet& in);
		//void craftSessReq(pcpp::Packet& in);
		//void craftKeepAlv(pcpp::Packet& in);
	};
	
};