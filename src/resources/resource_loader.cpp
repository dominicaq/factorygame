#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "resource_loader.h"

// Loaders
#include "objloader.h"

#include <fstream>
#include <sstream>
#include <iostream>

/*
* System
*/
std::string ResourceLoader::readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "ERROR::RESOURCE_LOADER::Failed to open file: " << filePath << "\n";
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

/*
* Images
*/
unsigned char* ResourceLoader::loadImage(const std::string& filePath, int* width, int* height, int* nrChannels) {
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(filePath.c_str(), width, height, nrChannels, 0);

    if (!data) {
        std::cerr << "ERROR::RESOURCE_LOADER::Failed to load image: " << filePath << "\n";
        return nullptr;
    }

    return data;
}

void ResourceLoader::freeImage(unsigned char* data) {
    stbi_image_free(data);
}

/*
* Mesh
*/
Mesh* ResourceLoader::loadMesh(const std::string& filepath) {
    std::string fileContent = readFile(filepath);
    if (fileContent.empty()) {
        std::cerr << "ERROR::RESOURCE_LOADER::File contents empty: " << filepath << "\n";
        return nullptr;
    }

    // Extract the file extension
    std::size_t dotPos = filepath.find_last_of(".");
    if (dotPos == std::string::npos) {
        std::cerr << "ERROR::RESOURCE_LOADER::Failed to determine file type for: " << filepath << "\n";
        return nullptr;
    }
    std::string extension = filepath.substr(dotPos + 1);

    // Convert extension to lowercase for comparison
    for (char& c : extension) {
        c = std::tolower(c);
    }

    // Determine the appropriate function based on the extension
    if (extension == "obj") {
        return ObjLoader::loadOBJ(fileContent);
    } else if (extension == "gltf") {
        std::cerr << "ERROR::RESOURCE_LOADER::GLTF loading not yet implemented: " << filepath << "\n";
        return nullptr;
    } else {
        std::cerr << "ERROR::RESOURCE_LOADER::Unknown file extension: " << extension << " for file: " << filepath << "\n";
        return nullptr;
    }
}
