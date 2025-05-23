#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <entt/entt.hpp>

#include "../renderer/material.h"

struct RawMeshData {
    std::vector<glm::vec4> packedTNBFrame;
    std::vector<glm::vec3> vertices;
    std::vector<uint32_t> indices;
    std::vector<glm::vec2> uvs;
    int drawMode = GL_TRIANGLES;

    // (TO BE MODFIED)
    Material* material;

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

struct Mesh {
    /*
    * WARNING: DO NOT set the Mesh ID yourself. It will be overwritten anyway.
    */
    size_t id = SIZE_MAX;

    // Optional overrides
    uint32_t count = 0;
    uint32_t instanceCount = 1;
    uint32_t firstIndex = 0;
    uint32_t baseVertex = 0;
    uint32_t baseInstance = 0;
    uint32_t materialIndex = 0xFFFFFFFF;
    int drawMode = GL_TRIANGLES;

    // (TO BE REMOVED)
    Material* material = nullptr;
};

struct MeshInstance {
    uint32_t materialIndex = 0xFFFFFFFF;
    uint8_t id;

    // Raw mesh data (TO BE REMOVED)
    Material* material;
};
