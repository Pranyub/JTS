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