#include "PacketParser.h"
#include "Util.h"
#include <UdpLayer.h>
#include <PayloadLayer.h>
#include <IPv4Layer.h>
#include <iostream>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/hmac.h>
#include "Config.h"
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
	
	uint8_t* message_pointer = payloadLayer->getData();
	udpInfo.message_len = payloadLayer->getDataLen();
	udpInfo.srcIP = ntohl(ipv4Layer->getSrcIpAddress().toInt());
	udpInfo.dstIP = ntohl(ipv4Layer->getDstIpAddress().toInt());
	udpInfo.srcPort = (int)udpLayer->getUdpHeader()->portSrc;
	udpInfo.dstPort = (int)udpLayer->getUdpHeader()->portDst;
	
	if (udpInfo.srcIP != cfg::switchIPAddrInt)
		return false;

	//Initialize raw with the payload data
	raw = new vector<uint8_t>;
	for (int i = 0; i < udpInfo.message_len; i++) {
		int a = *(message_pointer + i);
	}
	for (int i = 0; i < udpInfo.message_len; i++) {
		raw->push_back(*(message_pointer + i));
	}

	switch (raw->at(0))
	{
	case PIA_MSG:
		parsePia(*raw);
		break;
	case BROWSE_REQUEST:
		parseBrowseRequest();
		break;
	case BROWSE_REPLY:
		parseBrowseReply(); //Used to get session key (used for encryption/decryption)
		printf("Browse Reply from %x\n", udpInfo.srcIP);
		break;
	}
	
	return true;
}

bool Parser::parsePia(std::vector<uint8_t> piaMsg) {
	//Check if header matches
	for (int i = 0x00; i < 0x04; i++) {
		if (piaMsg[i] != header.magic[i])
			return false;
	}

	vector<uint8_t>::iterator iter = piaMsg.begin() + 5;
	iter = header.fill(iter);

	//The encrypted packet without unencrypted header
	vector<uint8_t> enc;
	while (iter < piaMsg.end())
		enc.push_back(*iter++);


	vector<uint8_t> dec;
	if(DecryptPia(enc, &dec))
		message.setMessage(dec);
	
	return true;
}

vector<uint8_t>::iterator Parser::PIAHeader::fill(vector<uint8_t>::iterator iter) {
	connID = *iter++;
	packetID = convertType(iter, 2); iter += 2;
	copy(iter, iter + 8, nonceCounter.data()); iter += 8;
	copy(iter, iter + 16, tag.data()); iter += 16;

	return iter;
}

std::vector<uint8_t> Parser::PIAHeader::set() {
	vector<uint8_t> out;
	out.insert(out.begin(), magic, magic + sizeof(magic));
	out.push_back(version);
	out.push_back(connID);
	std::vector<uint8_t> temp = NumToVector(packetID, 2);
	out.insert(out.end(), temp.begin(), temp.end());
	out.insert(out.end(), nonceCounter.begin(), nonceCounter.end());
	out.insert(out.end(), tag.begin(), tag.end());

	return out;
}

int Parser::Message::setMessage(vector<uint8_t> data) {
	vector<uint8_t>::iterator iter = data.begin();
	
	
	field_flags = *iter++;
	//set all message header values according to the field flags
	if (field_flags & 1)
		msg_flag = *iter++;
	if (field_flags & 2) {
		payload_size = convertType(iter, 2);
		iter += 2;
	}
	if (field_flags & 4) {
		protocol_type = *iter++;
		copy(iter, iter + 3, protocol_port);
		iter += 3;
	}
	if (field_flags & 8) {
		destination = convertType(iter, 8);
		iter += 8;
	}
	if (field_flags & 16) {
		source_station_id = convertType(iter, 8);
		iter += 8;
	}

	vector<uint8_t> temp(payload_size);
	for (int i = 0; i < payload_size; i++)
		temp[i] = *iter++;
	payload = temp;
	return iter - data.begin();
}

vector<uint8_t> Parser::Message::getMessage() {
	vector<uint8_t> out;
	vector<uint8_t> temp;

	out.push_back(field_flags);
	
	if (field_flags & 1)
		out.push_back(msg_flag);
	
	if (field_flags & 2) {
		temp = NumToVector(payload_size, 2);
		out.insert(out.end(), temp.begin(), temp.end());
	}
	
	if (field_flags & 4) {
		out.push_back(protocol_type);
		out.insert(out.end(), protocol_port, protocol_port + 3);
	}
	
	if (field_flags & 8) {
		temp = NumToVector(destination, 8);
		out.insert(out.end(), temp.begin(), temp.end());
	}
	
	if (field_flags & 16) {
		temp = NumToVector(source_station_id, 8);
		out.insert(out.end(), temp.begin(), temp.end());
	}
	
	return out;
}

void Parser::Message::appendHeader(vector<uint8_t>* data) {
	payload_size = data->size();
	vector<uint8_t> header = getMessage();
	data->insert(data->begin(), header.begin(), header.end());

}

bool Parser::parseBrowseRequest() {
	if (udpInfo.message_len != 873) { return false; } //Safety check; all browse request packets are 1360 bytes long
	
	message.protocol_type = LAN;
	message.payload.assign(raw->begin(), raw->end());
	message.payload_size = udpInfo.message_len;
	
	array<uint8_t, 12> challengeNonce;
	array<uint8_t, 16> challengeKey;
	array<uint8_t, 16> challengeTag;
	array<uint8_t, 256> challenge;
	
	challengeNonce[0] = 10;
	challengeNonce[1] = 0;
	challengeNonce[2] = 0;
	challengeNonce[3] = 255;

	copy(raw->begin() + 577, raw->begin() + 585, challengeNonce.data() + 4);
	copy(raw->begin() + 585, raw->begin() + 601, challengeKey.data());
	copy(raw->begin() + 601, raw->begin() + 617, challengeTag.data());
	copy(raw->begin() + 617, raw->begin() + 873, challenge.data());

	int len;
	uint8_t decryptedKey[16];

	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), nullptr, nullptr, nullptr);
	EVP_EncryptInit_ex(ctx, nullptr, nullptr, GAME_KEY, nullptr);
	EVP_EncryptUpdate(ctx, decryptedKey, &len, challengeKey.data(), 16);

	EVP_CIPHER_CTX_reset(ctx);

	array<uint8_t, 256> decrypted;

	ctx = EVP_CIPHER_CTX_new();
	EVP_DecryptInit_ex(ctx, EVP_aes_128_gcm(), nullptr, nullptr, nullptr);
	EVP_DecryptInit_ex(ctx, nullptr, nullptr, decryptedKey, challengeNonce.data());
	EVP_DecryptUpdate(ctx, decrypted.data(), &len, challenge.data(), challenge.size());
	EVP_DecryptFinal_ex(ctx, decrypted.data() + decrypted.size(), &len);
	EVP_CIPHER_CTX_reset(ctx);
	
	uint8_t* resp;

	resp = HMAC(EVP_sha256(), GAME_KEY, 16, decrypted.data(), decrypted.size(), nullptr, nullptr);

	
	
	uint8_t* encKeyPtr;
	vector<uint8_t> respPre;
	vector<uint8_t> selfKey;
	HexToVector("ff900b316a606564d898ffc3351302fd", &selfKey);
	respPre.insert(respPre.end(), selfKey.begin(), selfKey.end());
	respPre.insert(respPre.end(), challengeKey.begin(), challengeKey.end());
	encKeyPtr = HMAC(EVP_sha256(), GAME_KEY, 16, respPre.data(), respPre.size(), nullptr, nullptr);


	array<uint8_t, 16> encKey;
	for (int i = 0; i < encKey.size(); i++)
		encKey[i] = *(encKeyPtr + i);

	array<uint8_t, 16> out;

	ctx = EVP_CIPHER_CTX_new();
	EVP_EncryptInit_ex(ctx, EVP_aes_128_gcm(), nullptr, nullptr, nullptr);
	EVP_EncryptInit_ex(ctx, nullptr, nullptr, encKey.data(), challengeNonce.data());
	EVP_EncryptUpdate(ctx, out.data(), &len, resp, 16);


	if (EVP_EncryptFinal_ex(ctx, out.data() + out.size(), &len) != 1) {
		printf("Error in Encryption\n");
		return false;
	}

	EVP_CIPHER_CTX_free(ctx);
	for (int i = 0; i < 16; i++)
		printf("%02x", challengeKey[i]);
	printf("\n");
	
	return true;
}

bool Parser::parseBrowseReply() {

	if (udpInfo.message_len != 1360) { return false; } //Safety check; all browse reply packets are 1360 bytes long
										 //Checking for a matching session id is not yet implemented, so some errors may arise when attempting to use this program in a room with more than two switches

	message.protocol_type = LAN;
	message.payload.assign(raw->begin(), raw->end());
	message.payload_size = udpInfo.message_len;

	uint8_t session_param[32];
	for (int i = 0; i < 32; i++) {
		session_param[i] = raw->at(i + 1270);
	}
	session_param[31] += 1;
	
	setSessionKey(session_param); //This is all we care about

	for (int i = 0; i < 4; i++) {
		sessionID[i] = raw->at(i + 9);
	}
	return true;
}

void Parser::setSessionKey(const uint8_t mod_param[]) //creates hash of the given array and sets session key to it
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

bool Parser::DecryptPia(const std::vector<uint8_t> encrypted, std::vector<uint8_t> *decrypted) {
	
	//If this bit is set the packet isn't encrypted
	if (header.version >> 7 == 0)
		return true;
	else if (!decryptable)
		return false; //Cannot decrypt if sessionKey isn't set

	uint8_t nonce[12];

	//Set the nonce with the source ip and nonce counter
	for (int i = 0; i < 4; i++)
		nonce[i] = (udpInfo.srcIP >> (24 - i * 8) & 0xFF);
	nonce[4] = header.connID;
	for (int i = 1; i < 8; i++) {
		nonce[i + 4] = header.nonceCounter[i];
	}

	int decrypted_len;
	decrypted->resize(encrypted.size(), 0);

	//Start decryption
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	EVP_DecryptInit_ex(ctx, EVP_aes_128_gcm(), nullptr, nullptr, nullptr);
	EVP_DecryptInit_ex(ctx, nullptr, nullptr, sessionKey.data(), nonce);
	EVP_DecryptUpdate(ctx, decrypted->data(), &decrypted_len, encrypted.data(), encrypted.size());
	EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, header.tag.data());


	if (EVP_DecryptFinal_ex(ctx, decrypted->data() + decrypted_len, &decrypted_len) == 0) {
		EVP_CIPHER_CTX_free(ctx);
		return false;
	}

	EVP_CIPHER_CTX_free(ctx);
	/*
	printf("RECV: ");
	for (int i : *decrypted)
		printf("%02x ", i);
	printf("\n\n");
	*/

	return true;

}

bool Parser::EncryptPia(std::vector<uint8_t> decrypted, std::vector<uint8_t>* encrypted, PIAHeader header_self) {
	if (!decryptable) return false; //Cannot encrypt if sessionKey isn't set

	while (decrypted.size() % 16 != 0)
		decrypted.push_back(0xff); //add padding

	encrypted->resize(decrypted.size());
	int enc_len;
	//Start encryption
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	EVP_EncryptInit_ex(ctx, EVP_aes_128_gcm(), nullptr, nullptr, nullptr);
	EVP_EncryptInit_ex(ctx, nullptr, nullptr, sessionKey.data(), header_self.nonceCounter.data());
	EVP_EncryptUpdate(ctx, encrypted->data(), &enc_len, decrypted.data(), decrypted.size());


	if (EVP_EncryptFinal_ex(ctx, encrypted->data() + decrypted.size(), &enc_len) != 1) {
		printf("Error in Encryption\n");
		return false;
	}

	EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, header_self.tag.data());
	EVP_CIPHER_CTX_free(ctx);

	*header_self.nonce += 1;
	vector<uint8_t> temp = NumToVector(*header_self.nonce, sizeof(*header_self.nonce));
	for (int i = 0; i < temp.size(); i++) {
		header_self.nonceCounter[i] = temp[i];
	}

	temp = header_self.set();
	encrypted->insert(encrypted->begin(), temp.begin(), temp.end());
	
	/*
	printf("SENT: ");
	for (int i : decrypted)
		printf("%02x ", i);
	printf("\n\n");
	*/

	return true;
}

void Parser::resetAll() {
	delete(raw);
	raw = nullptr;
	udpInfo = udpInfoReset;
	header = headerReset;
	message = messageReset;
}