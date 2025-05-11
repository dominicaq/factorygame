#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include "resource_loader.h"

// Loaders
#include "objloader.h"
#include "parsers/gltf/gltfParser.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <utility>

/*
* System
*/
std::string ResourceLoader::readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::filesystem::path absolutePath = std::filesystem::absolute(filePath);
        std::cerr << "[Error] ResourceLoader::readFile: Failed to open file: "
                 << filePath << "\n";
        std::cerr << "[Error] Absolute path was: " << absolutePath.string() << "\n";
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
        std::cerr << "[Error] ResourceLoader::loadImage: Failed to load image: " << filePath << "\n";
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
        std::cerr << "[Error] ResourceLoader::loadMesh: File contents empty: " << filepath << "\n";
        return nullptr;
    }

    // Extract the file extension
    std::size_t dotPos = filepath.find_last_of(".");
    if (dotPos == std::string::npos) {
        std::cerr << "[Error] ResourceLoader::loadMesh: Failed to determine file type for: " << filepath << "\n";
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
        std::vector<Mesh> meshes;
        bool success = loadGltf(meshes, filepath);
        if (!success ||meshes.empty()) {
            std::cerr << "[Error] ResourceLoader::loadMesh: Failed to load glTF file or file contains no meshes: " << filepath << "\n";
            return nullptr;
        }

        // Return just the first mesh for now
        Mesh* firstMesh = new Mesh(meshes[0]);
        return firstMesh;
    } else {
        std::cerr << "[Error] ResourceLoader::loadMesh: Unknown file extension: " << extension << " for file: " << filepath << "\n";
        return nullptr;
    }
}

/*
* Cube maps
*/
/*
* Cubemap (Loading only image data, no OpenGL calls)
*/
std::vector<unsigned char*> ResourceLoader::loadCubemapImages(const std::vector<std::string>& faces, int* width, int* height, int* nrChannels) {
    stbi_set_flip_vertically_on_load(false);

    std::vector<unsigned char*> data;
    for (const std::string& face : faces) {
        unsigned char* imgData = stbi_load(face.c_str(), width, height, nrChannels, 0);
        if (!imgData) {
            std::cerr << "[Error] ResourceLoader::loadCubemapImages: Failed to load image: " << face << "\n";
            freeCubemapImages(data);
            return {};
        }
        data.push_back(imgData);
    }
    return data;
}

void ResourceLoader::freeCubemapImages(std::vector<unsigned char*>& data) {
    for (unsigned char* img : data) {
        stbi_image_free(img);
    }
    data.clear();
}
