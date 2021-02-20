#include <cstdio>
#include "Config.h"
#include "Transciever.h"
#include <Packet.h>
#include <thread>
#include <fstream>
#include <iostream>

using namespace std;


int main(int argc, char* argv[])
{

	//JANK TRADING SYSTEM

	if (argc != 4) {
		printf("Usage: XTransrecieverPlus interface1 interface2 injectFile\n");
		exit(1);
	}

	
	string interfaceIPAddr1 = argv[1];
	string interfaceIPAddr2 = argv[2];
	string switchIP = argv[2];
	ifstream injectFile(argv[3], ios::out | ios::binary);


	Tx tx1(interfaceIPAddr1, switchIP, cfg::searchfilter, false);
	Tx tx2(interfaceIPAddr2, switchIP, cfg::searchfilter, true);
	
	tx1.cookie.output = tx2.dev;
	tx2.cookie.output = tx1.dev;

	tx1.cookie.otherSwitchMac = &tx2.cookie.selfSwitchMac;
	tx2.cookie.otherSwitchMac = &tx1.cookie.selfSwitchMac;

	std::array<uint8_t, 16> sessionKey;
	std::array<uint8_t, 16> fallbackKey;
	tx1.cookie.parser.linkSessionKeys(&sessionKey, &fallbackKey);
	tx2.cookie.parser.linkSessionKeys(&sessionKey, &fallbackKey);
	
	if (!injectFile.is_open()) {
		printf("Unable to find file '%s'.\n", argv[2]);
		exit(1);
	}
	
	Pokemon injectPokemon;
	printf("INJECT POKEMON: ");
	for (int i = 0; i < injectPokemon.data.size(); i++) {
		injectFile.read((char*)&injectPokemon.data[i], sizeof(uint8_t));
		printf("%02x", injectPokemon.data[i]);
	}
	printf("\n");

	tx1.cookie.injectPokemon.set(injectPokemon);
	tx2.cookie.injectPokemon.set(injectPokemon);


	tx1.Start();
	tx2.Start();


	
	while (true);
}