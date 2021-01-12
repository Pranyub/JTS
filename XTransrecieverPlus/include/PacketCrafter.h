#include "PacketParser.h"
#include <Packet.h>
namespace crft {

	struct PiaPacket {
		int protocolType = 0;
		int messageType = 0;
		
		Parser* parser;

		PiaPacket(Parser* parserIn) {
			parser = parserIn;
		}
		PiaPacket() {}
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

		

		Lan(Parser *parserIn) : PiaPacket(parserIn) {
			protocolType = 0x44;
		}
		Lan() {}
		pcpp::Packet craftBrowseReq();

	};
	
};