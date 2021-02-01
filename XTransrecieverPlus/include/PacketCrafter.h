#include "PacketParser.h"
#include <Packet.h>
#include <EthLayer.h>
#include <IPv4Layer.h>
#include <UdpLayer.h>
#include <PayloadLayer.h>
#include "Util.h"
#include "Config.h"

using namespace std;

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
		pcpp::Packet craftPacket(pcpp::Packet recieved, vector<uint8_t> data);
	};

	//All data structures are unused

	struct InetAddress {
		std::array<uint8_t, 16> addr;
		uint16_t port = 0xc001;
		uint8_t size = 0;
		InetAddress(uint8_t version) {
			if (version == 2)
				size = 6;
			else
				size = 12;
		}

		std::vector<uint8_t> getInetAddress() {
			vector<uint8_t> out;
			out.insert(out.end(), addr.begin(), addr.begin() + size);
			out.insert(out.end(), { (uint8_t)(port >> 8 & 0xff), (uint8_t)(port & 0xff) });
			return out;
		}

		void parseInetAddress(std::vector<uint8_t>::iterator iter, uint8_t size) {
			if (size == 2) {
				//std::copy(iter, iter + 6, addr);
				port = (uint16_t) *(iter + 7) << 8 + *(iter + 8);
			}
			else {
				//std::copy(iter, iter + 12, addr);
				port = (uint16_t) * (iter + 13) << 8 + *(iter + 14);
			}
		}
	};

	struct StationLocation {
		uint8_t pub_addr_size = 0x02;
		uint8_t local_addr_size = 0x06;
		InetAddress pub_addr{pub_addr_size};
		InetAddress local_addr{local_addr_size};
		uint64_t pid = 0;
		uint8_t nat = 0; //I have no idea what this is
		uint8_t type = 0;
		uint8_t probeinit = 0;
		uint8_t is_local = 1;

		vector<uint8_t> getStationLocation() {
			vector<uint8_t> out;
			out.insert(out.end(), { pub_addr_size, local_addr_size });
			vector<uint8_t> temp = pub_addr.getInetAddress();
			out.insert(out.end(), temp.begin(), temp.end());
			temp = local_addr.getInetAddress();
			out.insert(out.end(), temp.begin(), temp.end());
			temp = util::NumToVector(pid, sizeof(pid));
			out.insert(out.end(), temp.begin(), temp.end());
			out.insert(out.end(), {nat, type, probeinit, is_local});
			return out;
		}
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


		pcpp::Packet craftBrowseReq();
		pcpp::Packet craftBrowseRep();
		pcpp::Packet craftHostReq();
		pcpp::Packet craftSessReq();
		pcpp::Packet craftKeepAlive();
	};
	
	struct Station : PiaPacket {
		enum MSG_TYPES {
			CONN_REQ = 1,
			CONN_REP = 2,
			DC_REQ = 3,
			DC_REP = 4,
			ACK = 5,
		};

		Station(Parser* parserIn) : PiaPacket(parserIn) {
			message.protocol_type =	STATION;
			message.msg_flag = 0x01;
		}
		Station() {};
		pcpp::Packet craftConnReq();
		pcpp::Packet craftConnRep();
		pcpp::Packet craftDcReq();
		pcpp::Packet craftDcRep();
		pcpp::Packet craftAck();
	};
};