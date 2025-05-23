#pragma once

#include "../components/mesh.h"
#include "scene/sceneData.h"

#include <string>
#include <memory>
#include <vector>

namespace ResourceLoader {
    /*
    * System
    */
    std::string readFile(const std::string& filePath);

    /*
    * Mesh functions
    */
    RawMeshData* loadMesh(const std::string& filepath);

    void loadMeshVector(const std::string& filepath,
                        std::vector<std::unique_ptr<RawMeshData>>& meshes,
                        std::vector<std::unique_ptr<MaterialDefinition>>& materialDefs,
                        std::vector<SceneData>& nodeData);

    /*
    * Image functions
    */
    unsigned char* loadImage(const std::string& filePath, int* width, int* height, int* nrChannels);
    void freeImage(unsigned char* data);

    /*
    * Cubemap loaders
    */
    std::vector<unsigned char*> loadCubemapImages(const std::vector<std::string>& faces, int* width, int* height, int* nrChannels);
    void freeCubemapImages(std::vector<unsigned char*>& data);
}
