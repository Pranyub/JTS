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
	
	hasResp = false;

	for (Parser::Message message : parser->messageVector) {
		if (parser->recv_message.payload.size() < 1)
			break;
		msgType = parser->recv_message.payload[0];
		if (protocol == 0x7c) {
			if (msgType == 0x07)
				printf("FOUND A 7!!!!!!! %02x%02x", parser->recv_message.payload[2], parser->recv_message.payload[3]);
		}


	}
	protocol = parser->recv_message.protocol_type;
	
	
	
	
	

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
	default:
		hasResp = false;
		break;
	}

}