#include <cstdio>
#include "Reader.h"
#include "Config.h"

using namespace std;

int main(int argc, char* argv[])
{
	Reader r;
	const string path = "D:\\ninjhax\\main\\Documents - HDD\\GitHub\\XTransrecieverPlus\\packets\\";
	if (!cfg::is_live)
		r.Start(path + "XTransreciever_16.pcap");

	return 0;
}