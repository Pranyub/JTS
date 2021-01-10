#include "PacketParser.h"
#include "Util.h"
#include <UdpLayer.h>
#include <PayloadLayer.h>
#include <IPv4Layer.h>
#include <iostream>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/aes.h>

using namespace pcpp;
using namespace std;
using namespace util;

bool Parser::onPacket(Packet packet) {
	
	resetAll();

	UdpLayer* udpLayer = packet.getLayerOfType<UdpLayer>();
	IPv4Layer* ipv4Layer = packet.getLayerOfType<IPv4Layer>();
	PayloadLayer* payloadLayer = packet.getLayerOfType<PayloadLayer>();
	
	if (udpLayer == nullptr || payloadLayer == nullptr || ipv4Layer == nullptr)
		return false;

	udpInfo.srcIP = ntohl(ipv4Layer->getSrcIpAddress().toInt());
	udpInfo.dstIP = ntohl(ipv4Layer->getDstIpAddress().toInt());
	udpInfo.srcPort = (int)udpLayer->getUdpHeader()->portSrc;
	udpInfo.dstPort = (int)udpLayer->getUdpHeader()->portDst;

	udpInfo.message_len = payloadLayer->getPayloadLen();
	uint8_t* message_pointer = payloadLayer->getData();
	
	//Initialize raw with the payload data
	for (int i = 0; i < udpInfo.message_len; i++) {
		raw.push_back(*(message_pointer + i)); 
	}

	switch (raw[0])
	{
	case PIA_MSG:
		parsePia(raw);
		break;
	case BROWSE_REQUEST:
		break;
	case BROWSE_REPLY:
		parseBrowseReply(); //Used to get session key (used for encryption/decryption)
		printf("Browse Reply from %x\n", udpInfo.srcIP);
		break;
	}
	return true;
}

bool Parser::parsePia(vector<uint8_t> raw) {
	//Check if header matches
	for (int i = 0x00; i < 0x04; i++) {
		if (raw[i] != header.magic[i])
			return false;
	}

	vector<uint8_t>::iterator iter = raw.begin() + 5;
	iter = header.fill(iter);
	return true;
}

std::vector<uint8_t>::iterator Parser::PIAHeader::fill(std::vector<uint8_t>::iterator iter) {
	connID = *iter++;
	packetID = convertType(iter, 2); iter += 2;
	copy(iter, iter + 8, nonceCounter.data()); iter += 8;
	copy(iter, iter + 16, tag.data()); iter += 16;

	return iter;
}

bool Parser::parseBrowseReply() {

	if (udpInfo.message_len != 1360) { return false; } //Safety check; all browse reply packets are 1360 bytes long
										 //Checking for a matching session id is not yet implemented, so some errors may arise when attempting to use this program in a room with more than two switches

	uint8_t session_param[32];
	for (int i = 0; i < 32; i++) {
		session_param[i] = raw[i + 1270];
	}
	session_param[31] += 1;
	
	setSessionKey(session_param); //This is all we care about

	for (int i = 0; i < 4; i++) {
		sessionID[i] = raw[i + 9];
	}
	return true;
}

void Parser::setSessionKey(uint8_t mod_param[]) //creates hash of the given array and sets session key to it
{
	HMAC_CTX* ctx = HMAC_CTX_new();
	unsigned int hmac_len;
	uint8_t session_key_ext[32] = {};
	HMAC_Init_ex(ctx, GAME_KEY, 16, EVP_sha256(), nullptr);

	HMAC_Update(ctx, mod_param, 32);
	HMAC_Final(ctx, session_key_ext, &hmac_len);

	//set actual sessionKey equal to first 16 bytes of the full key.
	for (int i = 0; i < 16; i++) {
		sessionKey[i] = session_key_ext[i];
	}
	decryptable = true;
}

void Parser::resetAll() {
	raw.clear();
	udpInfo = udpInfoReset;
	header = headerReset;
	message = messageReset;
}