#include <cstdio>
#include "Reader.h"
#include "Config.h"
#include "Transciever.h"

using namespace std;

int main(int argc, char* argv[])
{
	Reader r;
	Tx tx;

	const string path = "D:\\ninjhax\\main\\Documents - HDD\\GitHub\\XTransrecieverPlus\\packets\\";
	if (!cfg::is_live)
		r.Start(path + cfg::fileName);
	else {
		tx.Start(cfg::interfaceIPAddr, cfg::switchIPAddr, cfg::searchfilter);
		while (true) {}
	}
	return 0;
}