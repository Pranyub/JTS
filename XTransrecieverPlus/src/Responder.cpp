#include "Responder.h"
#include <PayloadLayer.h>


using namespace std;
using namespace pcpp;
using namespace crft;


void Responder::setParser(Parser& parserIn) {
	parser = &parserIn;
	lan = Lan(parser);
}

bool Responder::getResp(pcpp::Packet& out) {
	protocol = parser->message.protocol_type;
	if (parser->message.payload.size() < 1)
		return false;
	msgType = parser->message.payload[0];
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

void Responder::parseLan(Packet& packet) {
	switch (msgType)
	{
	case Lan::BROWSE_REQ:
		lan.craftBrowseReq(packet);
		break;
	default:
		hasResp = false;
		break;
	}

}