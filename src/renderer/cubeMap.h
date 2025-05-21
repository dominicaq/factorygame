#pragma once

#include <vector>
#include <string>

namespace CubeMap {
    unsigned int createFromImages(const std::vector<std::string>& faces);
    unsigned int createDepthMap(int mapSize);
}

