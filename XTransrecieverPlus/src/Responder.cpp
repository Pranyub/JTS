#include "Responder.h"



using namespace std;
using namespace pcpp;
using namespace crft;


void Responder::setParser(Parser* parserIn) {
	parser = parserIn;
	lan = Lan(parser);
}

bool Responder::getResp(pcpp::Packet *out) {
	protocol = parser->message.protocol_type;
	if (parser->message.payload.size() < 1)
		return false;
	msgType = parser->message.payload[0];
	packet = out;
	bool hasResp = true;
	if (stage == HANDSHAKE) {
		switch (protocol)
		{
		case LAN:
			parseLan();
			break;
		default:
			hasResp = false;
			break;
		}
	}

	return hasResp;
}

void Responder::parseLan() {
	
	switch (msgType)
	{
	case Lan::BROWSE_REQ:
		packet = &lan.craftBrowseReq();
		break;
	default:
		break;
	}
	
}