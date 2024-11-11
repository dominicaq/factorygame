#pragma once

#include <bitset>
#include <string>

struct MetaData {
    std::string name;
    std::bitset<64> layers;
};
