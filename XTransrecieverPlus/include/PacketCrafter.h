#include "PacketParser.h"
#include <Packet.h>
class Crafter {
public:

	struct PiaPacket {
		int protocolType = 0;
		int messageType = 0;
		
		Parser* parser;

		PiaPacket(Parser* parserIn) {
			parser = parserIn;
		}

	private:
		pcpp::Packet craftPacket(int srcPort, int dstPort, int dstIP=0);
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

		pcpp::Packet craftBrowseReq();

	};
	
};