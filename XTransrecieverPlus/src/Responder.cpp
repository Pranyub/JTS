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
	
	for (int i = 0; i < parser->messageVector.size(); i++) {
		
		if (parser->messageVector[i].payload.size() < 1)
			break;
		protocol = parser->messageVector[i].protocol_type;
		msgType = parser->messageVector[i].payload[0];
		

		if (protocol == 0x7c) {
			//exit(1);
			if (msgType == 0x07) {
				printf("FOUND 7c:07!!!!! %02x%02x", parser->messageVector[i].payload[2], parser->messageVector[i].payload[3]);
			}
				
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
	default:
		hasResp = false;
		break;
	}

}