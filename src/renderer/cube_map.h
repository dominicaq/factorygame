#ifndef CUBE_MAP_H
#define CUBE_MAP_H

#include <vector>
#include <string>

namespace CubeMap {
    unsigned int createFromImages(const std::vector<std::string>& faces);
}

#endif // CUBE_MAP_H
