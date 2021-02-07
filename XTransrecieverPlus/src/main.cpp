#include <cstdio>
#include "Config.h"
#include "Transciever.h"
#include <Packet.h>
#include <thread>

using namespace std;


int main(int argc, char* argv[])
{

	//JANK TRADING SYSTEM


	const string path = "D:/ninjhax/main/Documents - HDD/GitHub/XTransrecieverPlus/packets/";
	
	Tx tx1;
	Tx tx2;
	
	tx1.cookie.output = tx2.dev;
	tx2.cookie.output = tx1.dev;

	tx1.Start(cfg::interfaceIPAddr1, cfg::switchIP, cfg::searchfilter, false);
	tx2.Start(cfg::interfaceIPAddr2, cfg::switchIP, cfg::searchfilter, true);

	
	while (true);
}