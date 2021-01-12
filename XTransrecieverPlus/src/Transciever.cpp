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
	Parser* parser = &cookie->parser;
	Responder* responder = &cookie->responder;

	Packet out;
	if (parser->onPacket(packet)) {
		printf("GOT PACKET!\n");
		if (responder->getResp(&out))
			dev->sendPacket(&out);
	}
}


void Tx::Start(const std::string interfaceIPAddr, const std::string switchIPAddr, const std::string searchfilter) {

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
	Cookie cookie;
	cookie.responder.setParser(cookie.parser);
	dev->startCapture(onPacket, &cookie);
}