#include <cstdio>
#include "Reader.h"
#include "Config.h"
int main(int argc, char* argv[])
{
	Reader r;
	if (cfg::is_live)
		r.Start("Packets.pcap");

	return 0;
}