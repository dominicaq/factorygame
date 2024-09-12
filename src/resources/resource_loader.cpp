#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "resource_loader.h"

#include <fstream>
#include <sstream>
#include <iostream>

// Forward declarations
static Mesh* loadOBJ(const std::string& filepath);
static void parseOBJLine(const std::string& line, Mesh& mesh);

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
    // Find the position of the last dot in the file path
    std::size_t dotPos = filepath.find_last_of(".");
    if (dotPos == std::string::npos) {
        std::cerr << "ERROR::RESOURCE_LOADER::Failed to determine file type for: " << filepath << "\n";
        return nullptr;
    }

    // Extract the file extension
    std::string extension = filepath.substr(dotPos + 1);

    // Convert extension to lowercase for comparison
    for (char& c : extension) {
        c = std::tolower(c);
    }

    // Determine the appropriate function based on the extension
    if (extension == "obj") {
        return loadOBJ(filepath);
    } else if (extension == "gltf") {
        std::cerr << "ERROR::RESOURCE_LOADER::GLTF loading not yet implemented: " << filepath << "\n";
        return nullptr;
    } else {
        std::cerr << "ERROR::RESOURCE_LOADER::Unknown file extension: " << extension << " for file: " << filepath << "\n";
        return nullptr;
    }
}

/*
 * OBJ Parsing
 */
static Mesh* loadOBJ(const std::string& filepath) {
    std::string fileContent = ResourceLoader::readFile(filepath);
    if (fileContent.empty()) {
        std::cerr << "ERROR::RESOURCE_LOADER::Failed to read OBJ file: " << filepath << "\n";
        return nullptr;
    }

    Mesh* mesh = new Mesh();
    std::istringstream fileStream(fileContent);
    std::string line;
    while (std::getline(fileStream, line)) {
        parseOBJLine(line, *mesh);
    }

    return mesh;
}

static void parseOBJLine(const std::string& line, Mesh& mesh) {
    std::istringstream sstream(line);
    std::string type;
    sstream >> type;

    if (type == "v") {
        // Vertex position
        glm::vec3 vertex;
        sstream >> vertex.x >> vertex.y >> vertex.z;
        mesh.vertices.push_back(vertex);
    } else if (type == "vt") {
        // Texture coordinates (UVs)
        glm::vec2 uv;
        sstream >> uv.x >> uv.y;
        mesh.uvs.push_back(uv);
    } else if (type == "vn") {
        // Vertex normal
        glm::vec3 normal;
        sstream >> normal.x >> normal.y >> normal.z;
        mesh.normals.push_back(normal);
    } else if (type == "f") {
        // Face
        unsigned int vertexIndex[3] = {0}, uvIndex[3] = {0}, normalIndex[3] = {0};
        std::string vertexData;

        // Process the 3 vertices that make up a triangle face
        for (int i = 0; i < 3; i++) {
            sstream >> vertexData;
            std::stringstream faceStream(vertexData);
            std::string indexToken;
            size_t count = 0;

            // Parse the vertex/uv/normal indices
            while (std::getline(faceStream, indexToken, '/')) {
                if (count == 0 && !indexToken.empty()) {
                    vertexIndex[i] = std::stoi(indexToken) - 1;  // Vertex index (1-based to 0-based)
                } else if (count == 1 && !indexToken.empty()) {
                    uvIndex[i] = std::stoi(indexToken) - 1;      // UV index (1-based to 0-based)
                } else if (count == 2 && !indexToken.empty()) {
                    normalIndex[i] = std::stoi(indexToken) - 1;  // Normal index (1-based to 0-based)
                }
                count++;
            }

            // Store vertex, UV, and normal indices
            mesh.indices.push_back(vertexIndex[i]);
        }
    }
}
