#include "Transciever.h"
#include <PcapFilter.h>
#include <Layer.h>
#include <EthLayer.h>
#include <IPv4Layer.h>
#include <PayloadLayer.h>
#include <UdpLayer.h>
#include <PlatformSpecificUtils.h>

using namespace pcpp;
using namespace std;


static void onPacket(RawPacket* rawPacket, PcapLiveDevice* dev, void* c)
{
	Tx::Cookie* cookie = (Tx::Cookie*)c;
	Packet packet = Packet(rawPacket);
	Parser *parser = &cookie->parser;
	Responder* responder = &cookie->responder;
	vector<Packet> outVector;

	cookie->packet = packet;
	cookie->isReady = true;
	
	if (parser->onPacket(packet)) {
		
		if (responder->getResp(outVector)) {
		}
		
	}
}



void Tx::Start(const std::string interfaceIPAddr, const std::string switchIPAddr, const std::string searchfilter, const bool secondary) {

	dev = PcapLiveDeviceList::getInstance().getPcapLiveDeviceByIp(interfaceIPAddr.c_str());

	if (dev == NULL)
	{
		printf("Cannot find interface with IPv4 address of '%s'\n", interfaceIPAddr.c_str());
		exit(1);
	}
	if (!dev->open())
	{
		printf("Cannot open device\n");
		exit(1);
	}

	if (!dev->setFilter(searchfilter))
	{
		printf("Cannot set filter '%s'\n", searchfilter);
		exit(1);
	}
	
	if (!secondary)
		dev->setFilter(IPFilter(switchIPAddr, SRC));
	else
		dev->setFilter(NotFilter(&IPFilter(switchIPAddr, SRC)));

	cookie.responder.setParser(cookie.parser);
	dev->startCapture(onPacket, &cookie);
}