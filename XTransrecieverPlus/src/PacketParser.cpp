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

	//The raw packet data (payload only)
	raw = new vector<uint8_t>;

	//Get info from packet
	UdpLayer* udpLayer = packet.getLayerOfType<UdpLayer>();
	IPv4Layer* ipv4Layer = packet.getLayerOfType<IPv4Layer>();
	PayloadLayer* payloadLayer = packet.getLayerOfType<PayloadLayer>();
	
	//make sure all fields are set, if they aren't, then quit
	if (udpLayer == nullptr || payloadLayer == nullptr || ipv4Layer == nullptr)
		return false;
	
	uint8_t* message_pointer = payloadLayer->getData();
	udpInfo.message_len = payloadLayer->getDataLen();
	
	//sender data
	udpInfo.srcIP = ntohl(ipv4Layer->getSrcIpAddress().toInt());
	udpInfo.dstIP = ntohl(ipv4Layer->getDstIpAddress().toInt());
	udpInfo.srcPort = (int)udpLayer->getUdpHeader()->portSrc;
	udpInfo.dstPort = (int)udpLayer->getUdpHeader()->portDst;
	

	//Initialize raw with the payload data
	for (int i = 0; i < udpInfo.message_len; i++) {
		raw->push_back(*(message_pointer + i));
	}

	//Differentiate PIA and BrowseRep/BrowseReq
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
		printf("Browse Reply from %x | session key: ", udpInfo.srcIP);
		for (int i : *sessionKey)
			printf("%02x", i);
		printf(" | ");
		for (int i : *fallbackSessionKey)
			printf("%02x", i);
		printf("\n");
		break;
	}
	
	return true;
}

bool Parser::parsePia(std::vector<uint8_t> piaMsg) {

	//Check if header matches. If it doesn't, then quit.
	for (int i = 0x00; i < 0x04; i++) {
		if (piaMsg[i] != recv_header.magic[i])
			return false;
	}

	vector<uint8_t>::iterator iter = piaMsg.begin() + 5;
	iter = recv_header.fill(iter);
	//The encrypted packet without unencrypted header
	vector<uint8_t> enc;
	while (iter != piaMsg.end())
		enc.push_back(*iter++);


	
	messageVector.clear();

	if (DecryptPia(enc, &dec)) {
		int offset = 0;

		/*for (int i : dec)
			printf("%02x", i);
		printf("\n");
		*/

		while (offset < dec.size()) {

			offset = recv_message.setMessage(dec, offset);
			if (offset == -1)
				break;
			else if (offset == -2)
				offset++;
			else
				messageVector.push_back(recv_message);
		}
			
		
	}



	return true;
}


vector<uint8_t>::iterator Parser::PIAHeader::fill(vector<uint8_t>::iterator iter) {
	connID = *iter++;
	packetID = convertType(iter, 2); iter += 2;
	headerNonce.resize(8);
	copy(iter, iter + 8, headerNonce.data()); iter += 8;
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
	out.insert(out.end(), headerNonce.begin(), headerNonce.end());
	out.insert(out.end(), tag.begin(), tag.end());

	return out;
}


int Parser::Message::setMessage(vector<uint8_t> data, int offset) {
	vector<uint8_t>::iterator iter = data.begin() + offset;
	

	field_flags = *iter++;

	//this might be pia header padding
	if (field_flags == 0xff) {
		return -1;
	}

	//this might be pia message padding
	if (field_flags == 0x00) {
		return -2;
	}

	//not enough room for a message and probably junk (?)
	if (data.end() - 24 < iter) {	
		return -1;
	}
	
	//set all message header values according to the field flags
	if (field_flags & 1 && iter + 1 < data.end())
		msg_flag = *iter++;
	if (field_flags & 2 && iter + 2 < data.end()) {
		payload_size = convertType(iter, 2);
		iter += 2;
	}
	if (field_flags & 4 && iter + 4 < data.end()) {
		protocol_type = *iter++;
		copy(iter, iter + 3, protocol_port);
		iter += 3;
	}
	if (field_flags & 8 && iter + 8 < data.end()) {
		destination = convertType(iter, 8);
		iter += 8;
	}
	if (field_flags & 16 && iter + 8 < data.end()) {
		source_station_id = convertType(iter, 8);
		iter += 8;
	}

	vector<uint8_t> temp(payload_size);
	for (int i = 0; i < payload_size; i++) {
		if (iter != data.end())
			temp[i] = *iter++;
	}
	payload = temp;
	
	//safety check (is it necessary?)
	if (payload.size() == 0)
		payload.push_back(0xff);

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


vector<uint8_t> Parser::CryptoChallenge::makeChallenge() {
	vector<uint8_t> out;
	out.push_back(version);
	out.push_back(enabled);

	vector<uint8_t> nonceCounter = NumToVector(*nonce, sizeof(*nonce));

	array<uint8_t, 12> challengeNonce;
	for (int i = 0; i < 8; i++)
		challengeNonce[i + 4] = nonceCounter[i];

	challengeNonce[0] = 10;
	challengeNonce[1] = 0;
	challengeNonce[2] = 0;
	challengeNonce[3] = 255;

	out.insert(out.end(), challengeNonce.begin() + 4, challengeNonce.end());
	out.insert(out.end(), challengeKey.begin(), challengeKey.end());
	out.insert(out.end(), challengeTag.begin(), challengeTag.end());
	out.insert(out.end(), challenge.begin(), challenge.end());
	return out;
}

bool Parser::CryptoChallenge::parseChallenge(vector<uint8_t> raw, array<uint8_t, 12>* challengeNonce) {
	//nothing really of value in browseReply, so only browseRequest can be parsed.
	if (raw.size() != 873) { return false; }


	challengeNonce->at(0) = 10;
	challengeNonce->at(1) = 0;
	challengeNonce->at(2) = 0;
	challengeNonce->at(3) = 255;

	challenge.resize(256);

	copy(raw.begin() + 577, raw.begin() + 585, challengeNonce->data() + 4);
	copy(raw.begin() + 585, raw.begin() + 601, challengeKey.data());
	copy(raw.begin() + 601, raw.begin() + 617, challengeTag.data());
	copy(raw.begin() + 617, raw.begin() + 873, challenge.data());
	
	return true;
}

vector<uint8_t> Parser::CryptoChallenge::makeResponse() {

	vector<uint8_t> nonceCounter = NumToVector(*nonce, sizeof(*nonce));

	array<uint8_t, 12> challengeNonce;
	for (int i = 0; i < 8; i++)
		challengeNonce[i + 4] = nonceCounter[i];
	challengeNonce[0] = 0x0a;
	challengeNonce[1] = 0x00;
	challengeNonce[2] = 0x00;
	challengeNonce[3] = 0xff;

	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

	//raw response
	uint8_t resp[32];
	HMAC(EVP_sha256(), GAME_KEY, 16, challenge.data(), challenge.size(), resp, nullptr);

	vector<uint8_t> respKey;
	vector<uint8_t> selfKey2;
	HexToVector("98408530300f066d20cf8fafa062cd87", &selfKey2);
	selfKey = selfKey2;
	respKey.insert(respKey.end(), selfKey.begin(), selfKey.end());
	respKey.insert(respKey.end(), challengeKey.begin(), challengeKey.end());
	uint8_t encKey[32];
	HMAC(EVP_sha256(), GAME_KEY, 16, respKey.data(), respKey.size(), encKey, nullptr);


	//encrypt response
	array<uint8_t, 16> enc;
	int len;

	ctx = EVP_CIPHER_CTX_new();
	EVP_EncryptInit_ex(ctx, EVP_aes_128_gcm(), nullptr, encKey, challengeNonce.data());
	EVP_EncryptUpdate(ctx, enc.data(), &len, resp, 16);
	EVP_EncryptFinal_ex(ctx, enc.data() + enc.size(), &len);
	EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, challengeTag.data());
	EVP_CIPHER_CTX_free(ctx);


	vector<uint8_t> out;
	out.push_back(version);
	out.push_back(enabled);
	out.insert(out.end(), challengeNonce.begin() + 4, challengeNonce.end());
	out.insert(out.end(), selfKey.begin(), selfKey.end());
	out.insert(out.end(), challengeTag.begin(), challengeTag.end());
	out.insert(out.end(), enc.begin(), enc.end());

	return out;
}


bool Parser::parseBrowseRequest() {
	if (udpInfo.message_len != 873) { return false; } //Safety check; all browse request packets are 1360 bytes long
	
	recv_message.protocol_type = LAN;

	array<uint8_t, 12> challengeNonce;
	array<uint8_t, 16>* challengeKey = &browseReply.challengeKey;
	std::vector<uint8_t>* challenge = &browseReply.challenge;
	browseReply.parseChallenge(*raw, &challengeNonce);

	//
	//Decrypt the challenge
	//

	//Get decryption key
	int len;
	uint8_t decryptedKey[16];

	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), nullptr, GAME_KEY, nullptr);
	EVP_EncryptUpdate(ctx, decryptedKey, &len, challengeKey->data(), 16);
	EVP_CIPHER_CTX_reset(ctx);


	
	//decrypt using key
	array<uint8_t, 256> decrypted;

	ctx = EVP_CIPHER_CTX_new();
	EVP_DecryptInit_ex(ctx, EVP_aes_128_gcm(), nullptr, decryptedKey, challengeNonce.data());
	EVP_DecryptUpdate(ctx, decrypted.data(), &len, challenge->data(), challenge->size());
	EVP_DecryptFinal_ex(ctx, decrypted.data() + decrypted.size(), &len);
	EVP_CIPHER_CTX_free(ctx);
	
	recv_message.payload.assign(raw->begin(), raw->end());
	recv_message.payload_size = decrypted.size();
	
	browseReply.challenge.assign(decrypted.begin(), decrypted.end());
	return true;
}

bool Parser::parseBrowseReply() {

	if (udpInfo.message_len != 1360) { return false; } //Safety check; all browse reply packets are 1360 bytes long

	//Checking for a matching session id is not yet implemented, so some errors may arise
	//when attempting to use this program in a room with more than two switches

	recv_message.protocol_type = LAN;
	recv_message.payload.assign(raw->begin(), raw->end());
	recv_message.payload_size = udpInfo.message_len;

	uint8_t session_param[32];
	for (int i = 0; i < 32; i++) {
		session_param[i] = raw->at(i + 1270);
	}
	session_param[31] += 1;
	
	setSessionKey(session_param); //This is all we care about

	for (int i = 0; i < 4; i++) {
		sessionID[i] = raw->at(i + 9); //This too I guess
	}
	return true;
}

void Parser::linkSessionKeys(std::array<uint8_t, 16>* key, std::array<uint8_t, 16>* fallback) {
	delete(sessionKey);
	delete(fallbackSessionKey);
	sessionKey = key;
	fallbackSessionKey = fallback;
}

void Parser::setSessionKey(const uint8_t mod_param[]) //creates hash of the given array and sets session key to it
{
	
	HMAC_CTX* ctx = HMAC_CTX_new();
	unsigned int hmac_len;
	uint8_t session_key_ext[32] = {};

	HMAC(EVP_sha256(), GAME_KEY, 16, mod_param, 32, session_key_ext, nullptr);

	std::array<uint8_t, 16> oldKey;

	


	//set actual sessionKey equal to first 16 bytes of the full key.
	for (int i = 0; i < 16; i++) {
		oldKey.at(i) = sessionKey->at(i);
		sessionKey->at(i) = session_key_ext[i];
	}
	
	//set fallback key to old session key
	if (oldKey != *sessionKey) {
		for (int i = 0; i < 16; i++)
			fallbackSessionKey->at(i) = oldKey.at(i);
	}

	decryptable = true;
}

bool Parser::DecryptPia(const std::vector<uint8_t> encrypted, std::vector<uint8_t> *decrypted) {
	
	//If this bit is set the packet isn't encrypted
	if (recv_header.version >> 7 == 0)
		return true;
	else if (!decryptable)
		return false; //Cannot decrypt if sessionKey isn't set

	uint8_t nonce[12];


	//Set the nonce with the source ip and nonce counter
	for (int i = 0; i < 4; i++)
		nonce[i] = (udpInfo.srcIP >> (24 - i * 8) & 0xFF);
	nonce[4] = recv_header.connID;
	for (int i = 1; i < 8; i++) {
		nonce[i + 4] = recv_header.headerNonce[i];
	}

	int decrypted_len;
	decrypted->resize(encrypted.size(), 0);
	
	//Start decryption
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	EVP_DecryptInit_ex(ctx, EVP_aes_128_gcm(), nullptr, nullptr, nonce);
	EVP_DecryptInit_ex(ctx, nullptr, nullptr, sessionKey->data(), nullptr);

	if (EVP_DecryptUpdate(ctx, decrypted->data(), &decrypted_len, encrypted.data(), encrypted.size()) != 1)
		printf("DECRYPT UPDATE ERROR\n");
	
	//check if tag matches
	EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, recv_header.tag.data());

	if (EVP_DecryptFinal_ex(ctx, decrypted->data() + decrypted_len, &decrypted_len) != 1) {
	//decryption was unsuccessful; try with the fallback session key;

		EVP_CIPHER_CTX_free(ctx);
		EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
		EVP_DecryptInit_ex(ctx, EVP_aes_128_gcm(), nullptr, nullptr, nonce);
		EVP_DecryptInit_ex(ctx, nullptr, nullptr, fallbackSessionKey->data(), nullptr);

		if (EVP_DecryptUpdate(ctx, decrypted->data(), &decrypted_len, encrypted.data(), encrypted.size()) != 1)
			printf("DECRYPT UPDATE ERROR\n");
		
		EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, recv_header.tag.data());
		
	
		//if it still failed, then return false
		if (EVP_DecryptFinal_ex(ctx, decrypted->data() + decrypted_len, &decrypted_len) != 1) {
			
			EVP_CIPHER_CTX_free(ctx);
			return false;
		}

		//otherwise swap the two keys
		else {
			array<uint8_t, 16>* temp = sessionKey;
			sessionKey = fallbackSessionKey;
			fallbackSessionKey = temp;
		}
	}

	encryptionKey = *sessionKey;

	EVP_CIPHER_CTX_free(ctx);

	return true;

}

bool Parser::EncryptPia(std::vector<uint8_t> decrypted, std::vector<uint8_t>* encrypted, PIAHeader header_self) {
	if (!decryptable) return false; //Cannot encrypt if sessionKey isn't set

	while (decrypted.size() % 16 != 0)
		decrypted.push_back(0xff); //add padding

	//vector<uint8_t> nonceCounter = NumToVector(header_self.nonce, sizeof(header_self.nonce));

	//Set the encryption nonce
	vector<uint8_t> nonce;

	for (int i = 3; i >= 0; i--)
		nonce.push_back((uint8_t)(udpInfo.srcIP >> 8 * i) & 0xff);

	nonce.push_back(header_self.connID);

	for (int i=1; i < 8; i++)
		nonce.push_back(header_self.headerNonce[i]);

	encrypted->resize(decrypted.size());
	int enc_len;

	//Start encryption
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	EVP_EncryptInit_ex(ctx, EVP_aes_128_gcm(), nullptr, nullptr, nullptr);
	EVP_EncryptInit_ex(ctx, nullptr, nullptr, encryptionKey.data(), nonce.data());
	EVP_EncryptUpdate(ctx, encrypted->data(), &enc_len, decrypted.data(), decrypted.size());


	if (EVP_EncryptFinal_ex(ctx, encrypted->data() + decrypted.size(), &enc_len) != 1) {
		printf("Error in Encryption\n");
		return false;
	}

	array<uint8_t, 16> tag;

	if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag.data()) != 1) {
		printf("Error in Tag\n");
	}

	header_self.tag = tag;

	EVP_CIPHER_CTX_free(ctx);

	vector<uint8_t> temp = header_self.set();
	encrypted->insert(encrypted->begin(), temp.begin(), temp.end());


	return true;
}

void Parser::resetAll() {
	delete(raw);
	raw = nullptr;
	udpInfo = udpInfoReset;
	messageVector.clear();
}