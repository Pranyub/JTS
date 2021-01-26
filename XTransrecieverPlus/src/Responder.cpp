#include "Responder.h"
#include <PayloadLayer.h>


using namespace std;
using namespace pcpp;
using namespace crft;


void Responder::setParser(Parser& parserIn) {
	parser = &parserIn;
	lan = Lan(parser);
	station = Station(parser);
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
		case STATION:
			printf("\n%02", msgType);
			exit(1);
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
		
		packets.push_back(lan.craftBrowseRep());
		packets.push_back(lan.craftBrowseReq());
		packets.push_back(lan.craftKeepAlive());
		break;
	case Lan::BROWSE_REP:
		packets.push_back(lan.craftKeepAlive());
		packets.push_back(lan.craftHostReq());
		break;
	case Lan::GET_HOST_REP:
		packets.push_back(lan.craftKeepAlive());
		packets.push_back(station.craftConnReq());
		printf("\nHERE!!!!!!!!!\n\n\n");
	default:
		hasResp = false;
		break;
	}

}