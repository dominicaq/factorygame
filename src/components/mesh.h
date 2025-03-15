#ifndef MESH_H
#define MESH_H

#include <glm/glm.hpp>
#include "../renderer/material.h"

#include <vector>

struct Mesh {
    // Mesh data
    size_t id = SIZE_MAX;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> bitangents;
    std::vector<unsigned int> indices;
    Material* material = nullptr;
    bool wireframe = false;

    // Method to clear CPU-side mesh data
    void clearData() {
        vertices.clear();
        uvs.clear();
        normals.clear();
        indices.clear();

        // Resize vectors to zero
        vertices.shrink_to_fit();
        uvs.shrink_to_fit();
        normals.shrink_to_fit();
        indices.shrink_to_fit();
    }
};

struct MeshInstance {
    size_t id = SIZE_MAX;
    Material* material = nullptr;
};

#endif // MESH_H
