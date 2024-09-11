#ifndef RESOURCE_LOADER_H
#define RESOURCE_LOADER_H

#include "mesh.h"

#include <string>

namespace ResourceLoader {
    /*
    * System
    */
    std::string readFile(const std::string& filePath);

    /*
    * Mesh functions
    */
    Mesh* loadMesh(const std::string& filepath);

    /*
    * Image functions
    */
    unsigned char* loadImage(const std::string& filePath, int* width, int* height, int* nrChannels);
    void freeImage(unsigned char* data);
}

#endif // RESOURCE_LOADER_H
