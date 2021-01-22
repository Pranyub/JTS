#include "Responder.h"
#include <PayloadLayer.h>


using namespace std;
using namespace pcpp;
using namespace crft;


void Responder::setParser(Parser& parserIn) {
	parser = &parserIn;
	lan = Lan(parser);
}

bool Responder::getResp(vector<Packet>& out) {
	protocol = parser->recv_message.protocol_type;
	if (parser->recv_message.payload.size() < 1)
		return false;
	msgType = parser->recv_message.payload[0];
	hasResp = true;
	
	if (stage == HANDSHAKE) {
		switch (protocol)
		{
		case LAN:
			parseLan(out);
			break;
		default:
			hasResp = false;
			break;
		}
	}

	return hasResp;
}

void Responder::parseLan(vector<Packet>& packets) {
	
	switch (msgType)
	{
	case Lan::BROWSE_REQ:
		
		//lan.craftBrowseRep(packet);
		packets.push_back(lan.craftBrowseReq());
		packets.push_back(lan.craftKeepAlive());
		break;
	case Lan::BROWSE_REP:
		packets.push_back(lan.craftKeepAlive());
		packets.push_back(lan.craftHostReq());
		break;
	case Lan::GET_HOST_REP:
		exit(1);
	default:
		hasResp = false;
		break;
	}

}