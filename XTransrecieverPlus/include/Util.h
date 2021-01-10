#pragma once
#include <vector>
namespace util {

	//Converts [size] ints at a pointer into a uint64_t.  
	uint64_t convertType(std::vector<uint8_t>::iterator iter, int size);
}