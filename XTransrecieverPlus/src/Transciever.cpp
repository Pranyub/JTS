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


static void onPacket(RawPacket* rawPacket, PcapLiveDevice* dev, void* c) {
	Tx::Cookie* cookie = (Tx::Cookie*)c;
	Packet packet = Packet(rawPacket);
	Parser *parser = &cookie->parser;
	Responder* responder = &cookie->responder;

	vector<Packet> outVector;
	
	EthLayer* eth = packet.getLayerOfType<EthLayer>();

	if (!cookie->selfSwitchMac.isValid()) {
		cookie->selfSwitchMac = eth->getSourceMac();
		printf("GOT DST: {%s}\n", eth->getSourceMac().toString().c_str());
	}
	if (eth->getDestMac() != MacAddress("ff:ff:ff:ff:ff:ff"))
		eth->setDestMac(*cookie->otherSwitchMac);

	if (cookie->isSecondary)
		packet.getLayerOfType<EthLayer>()->setSourceMac(cookie->output->getMacAddress());
	else
		packet.getLayerOfType<EthLayer>()->setSourceMac(dev->getMacAddress());

	vector<uint8_t> packetData;
	vector<uint8_t> out;

	if (parser->onPacket(packet)) {
		packetData = parser->dec;
		//bool isSet = responder->setPokemonRaw(packetData, cookie->selfSwitchMac, cookie->selfPokemon, cookie->injectPokemon);
		if (parser->raw->at(0) != BROWSE_REPLY && parser->raw->at(0) != BROWSE_REQUEST) {

			if (parser->EncryptPia(packetData, &out, parser->recv_header)) {

				packet.getLayerOfType<PayloadLayer>()->setPayload(out.data(), out.size());

				if (out != *parser->raw && !cookie->isSecondary) {
					
					
					printf("MISMATCH\n");
					for (int i : parser->dec)
						printf("%02x", i);
					printf("\n\n");
					for (int i : *parser->raw)
						printf("%02x", i);
	
					printf("\n\n");
					for (int i : parser->recv_header.tag)
						printf("%02x", i);

					printf("\n\n");
					
				}
			}
		}
	}

	packet.computeCalculateFields();
	cookie->output->sendPacket(&packet);
	
	parser->resetAll();
}



Tx::Tx(const std::string interfaceIPAddr, const std::string switchIPAddr, const std::string searchfilter, const bool secondary) {

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
	std::string filter = searchfilter;
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
	
	printf("%s\n", dev->getName());
	
	cookie.responder.setParser(cookie.parser);
	cookie.isSecondary = secondary;
}

void Tx::Start() {
	dev->startCapture(onPacket, &cookie);
}