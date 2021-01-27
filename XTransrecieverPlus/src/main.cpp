#include <cstdio>
#include "Config.h"
#include "Transciever.h"

using namespace std;

int main(int argc, char* argv[])
{

	Tx tx1;
	Tx tx2;

	const string path = "D:/ninjhax/main/Documents - HDD/GitHub/XTransrecieverPlus/packets/";
	
	tx1.Start(cfg::interfaceIPAddr1, 0, cfg::searchfilter);
	tx2.Start(cfg::interfaceIPAddr2, 0, cfg::searchfilter);

}