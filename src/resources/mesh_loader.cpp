#include "mesh_loader.h"
#include <fstream>
#include <sstream>
#include <iostream>

// Load mesh from file based on file type (currently only OBJ)
Mesh* MeshLoader::loadMesh(const std::string& filepath, FileType type) {
    switch (type) {
        case FileType::OBJ:
            return loadOBJ(filepath);
        case FileType::GLTF:
            std::cerr << "GLTF loading not yet implemented." << std::endl;
            return nullptr;
        default:
            std::cerr << "Unknown file type." << std::endl;
            return nullptr;
    }
}

// Load OBJ files, parse each line and store the data in Mesh
Mesh* MeshLoader::loadOBJ(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return nullptr;
    }

    Mesh* mesh = new Mesh();
    std::string line;
    while (std::getline(file, line)) {
        parseOBJLine(line, *mesh);
    }

    file.close();
    return mesh;
}

// Parse each line in the OBJ file
void MeshLoader::parseOBJLine(const std::string& line, Mesh& mesh) {
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
