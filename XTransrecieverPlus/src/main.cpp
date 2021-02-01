#include <cstdio>
#include "Config.h"
#include "Transciever.h"
#include <Packet.h>
#include <thread>

using namespace std;

struct TranscieverData {
	Tx tx;
	Tx* rx;
	pcpp::Packet* packet;
};

void doProxy(TranscieverData* transciever) {
	while (true) {
		if (transciever->rx->cookie.isReady) {
			printf("HERE\n");
			pcpp::Packet out;
			transciever->rx->cookie.getPacket(out);
			transciever->tx.dev->sendPacket(&out);
		}
	}
	
}

int main(int argc, char* argv[])
{

	TranscieverData transciever1;
	TranscieverData transciever2;

	const string path = "D:/ninjhax/main/Documents - HDD/GitHub/XTransrecieverPlus/packets/";
	
	transciever1.tx.Start(cfg::interfaceIPAddr1, cfg::switchIP, cfg::searchfilter, false);
	transciever2.tx.Start(cfg::interfaceIPAddr2, cfg::switchIP, cfg::searchfilter, true);

	transciever1.packet = &transciever2.tx.cookie.packet;
	transciever2.packet = &transciever1.tx.cookie.packet;
	transciever1.rx = &transciever2.tx;
	transciever2.rx = &transciever1.tx;
	thread thread1(doProxy, &transciever1);    // spawn new thread that calls foo()
	thread thread2(doProxy, &transciever2);
	
	while (true);
}