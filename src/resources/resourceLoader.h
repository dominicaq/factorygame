#ifndef RESOURCE_LOADER_H
#define RESOURCE_LOADER_H

#include "../components/mesh.h"
#include "scene/sceneData.h"

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
    void loadMeshVector(const std::string& filepath, std::vector<Mesh*>& meshes, std::vector<SceneData>& nodeData, Shader* shader);

    /*
    * Image functions
    */
    unsigned char* loadImage(const std::string& filePath, int* width, int* height, int* nrChannels);
    void freeImage(unsigned char* data);

    /*
    * Cubemap loaders
    */
    std::vector<unsigned char*> loadCubemapImages(const std::vector<std::string>& faces, int* width, int* height, int* nrChannels);
    void freeCubemapImages(std::vector<unsigned char*>& data);;
}

#endif // RESOURCE_LOADER_H
