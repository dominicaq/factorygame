#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <entt/entt.hpp>

#include "../renderer/material.h"

struct MeshInstance {
    uint32_t materialIndex = 0xFFFFFFFF;
    uint8_t id;

    // Raw mesh data (TO BE WIPED)
    Material* material;
};

struct Mesh {
    uint32_t count = 0;
    uint32_t instanceCount = 0;
    uint32_t firstIndex = 0;
    uint32_t baseVertex = 0;
    uint32_t baseInstance = 0;
    uint32_t materialIndex = 0xFFFFFFFF;
    int drawMode = GL_TRIANGLES;

    /*
    * WARNING: DO NOT set the Mesh ID yourself. It will be overwritten anyway.
    */
    size_t id = SIZE_MAX;
    bool wireframe = false;

    // Raw mesh data (TO BE WIPED)
    std::vector<glm::vec4> packedTNBFrame;
    std::vector<glm::vec3> vertices;
    std::vector<uint32_t> indices;
    std::vector<glm::vec2> uvs;
    Material* material = nullptr;

    // Method to clear CPU-side mesh data
    void clearData() {
        vertices.clear();
        uvs.clear();
        packedTNBFrame.clear();

        // Resize vectors to zero
        vertices.shrink_to_fit();
        uvs.shrink_to_fit();
        packedTNBFrame.shrink_to_fit();
        indices.shrink_to_fit();
    }
};
