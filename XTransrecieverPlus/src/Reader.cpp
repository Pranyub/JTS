#include <PcapFileDevice.h>
#include <PlatformSpecificUtils.h>
#include <Packet.h>
#include <PcapLiveDeviceList.h>
#include "Reader.h"
#include "PacketParser.h"

using namespace std;
using namespace pcpp;

void Reader::Start(string file) {
	IFileReaderDevice* reader = IFileReaderDevice::getReader(file.c_str());

	// verify that a reader interface was created
	if (reader == NULL)
	{
		printf("Cannot determine reader for file type\n");
		exit(1);
	}
	if (!reader->open())
	{
		printf("Cannot open file for reading\n");
		exit(1);
	}

	printf("\nBeginning...\n");

	RawPacket rawPacket;
	Packet packet;
	Parser parser;
	int count = 0;
	
	while (reader->getNextPacket(rawPacket))
	{
		packet = Packet(&rawPacket);

		parser.onPacket(packet);

		count++;
	}

	printf("\nDone! Got %d Packets.\n", count);
}