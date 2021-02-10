#include "Transciever.h"
#include <PcapFilter.h>
#include <Layer.h>
#include <EthLayer.h>
#include <IPv4Layer.h>
#include <PayloadLayer.h>
#include <UdpLayer.h>
#include <EthLayer.h>
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

	if (parser->onPacket(packet)) {
		//TODO: Add check pokemon method		
	}
	printf("{%08s}\n", cookie->output->getMacAddress().toString().c_str());
	packet.getLayerOfType<EthLayer>()->setSourceMac(cookie->output->getMacAddress());
	cookie->output->sendPacket(&packet);
	
	
	parser->resetAll();
}



Tx::Tx(const std::string interfaceIPAddr, const std::string switchIPAddr, const std::string searchfilter, const bool secondary) {

	dev = PcapLiveDeviceList::getInstance().getPcapLiveDeviceByIp(interfaceIPAddr.c_str());

	printf("%s\n", dev->getName());
	

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
	string filter = searchfilter;
	//This is to prevent feedback; only search for switchIP on primary interface, and ignore switchIP on secondary.
	if (secondary)
		filter.append(" and (not (ip and src net " + switchIPAddr + "))");
	else
		filter.append(" and (ip and src net " + switchIPAddr +  ")");

	if (!dev->setFilter(filter))
	{
		printf("Cannot set filter '%s'\n", filter);
		exit(1);
	}
	
	
	cookie.responder.setParser(cookie.parser);
	
}

void Tx::Start() {
	dev->startCapture(onPacket, &cookie);
}