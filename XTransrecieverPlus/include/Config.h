#pragma once
#include <iostream>
#include <fstream>
#include <string>
namespace cfg {

	const std::string searchfilter = "((src or dst portrange 49151-49156) or (src or dst port 30000)) and (udp)";
};
