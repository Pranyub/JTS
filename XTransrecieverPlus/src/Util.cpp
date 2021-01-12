#include "Util.h"

using namespace util;
using namespace std;

uint64_t util::convertType(vector<uint8_t>::iterator iter, int size) {
	uint64_t out = 0;
	for (int i = size - 1; i >= 0; i--) {
		out += (uint64_t)*iter++ << (8 * i);
	}

	return out;
}

uint64_t util::convertType(array<uint8_t, 2048>::iterator iter, int size) {
	uint64_t out = 0;
	for (int i = size - 1; i >= 0; i--) {
		out += (uint64_t)*iter++ << (8 * i);
	}

	return out;
}

void util::HexToVector(const string hex, vector<uint8_t> *in) {

	
	for (unsigned int i = 0; i < hex.length(); i += 2) {
		string byteString = hex.substr(i, 2);
		uint8_t byte = (uint8_t)strtol(byteString.c_str(), nullptr, 16);
		in->push_back(byte);
	}
}