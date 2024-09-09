#include "mesh_loader.h"
#include <fstream>
#include <sstream>
#include <iostream>

bool MeshLoader::loadMesh(const std::string& filepath, FileType type, Mesh& mesh, int* status) {
    switch (type) {
        case FileType::OBJ:
            return loadOBJ(filepath, mesh, status);
        case FileType::GLTF:
            // Future implementation
            std::cerr << "GLTF loading not yet implemented." << std::endl;
            *status = -2; // Unimplemented feature
            return false;
        default:
            std::cerr << "Unknown file type." << std::endl;
            *status = -1; // Unknown file type
            return false;
    }
}

bool MeshLoader::loadOBJ(const std::string& filepath, Mesh& mesh, int* status) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        *status = -1; // File not found
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        parseOBJLine(line, mesh);
    }

    file.close();
    *status = 0; // Success
    return true;
}

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
    }
}

bool MeshLoader::loadGLTF(const std::string& filepath, Mesh& mesh, int* status) {
    // Placeholder for future implementation
    return false;
}
