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
};

struct EntityMeshDefinition {
    std::unique_ptr<RawMeshData> rawMeshData;
    std::unique_ptr<MaterialDefinition> materialDef;
    entt::entity entity;

    // Constructor to make creation easier
    EntityMeshDefinition(entt::entity ent) : entity(ent) {
        rawMeshData = std::make_unique<RawMeshData>();
        materialDef = std::make_unique<MaterialDefinition>();
    }

    // Move semantics
    EntityMeshDefinition(EntityMeshDefinition&&) = default;
    EntityMeshDefinition& operator=(EntityMeshDefinition&&) = default;

    // Delete copy
    EntityMeshDefinition(const EntityMeshDefinition&) = delete;
    EntityMeshDefinition& operator=(const EntityMeshDefinition&) = delete;
};

struct InstancedMeshGroup {
    std::unique_ptr<RawMeshData> meshData;
    std::unique_ptr<MaterialDefinition> materialDef;
    std::vector<entt::entity> entities;
    Mesh* initializedMesh = nullptr;

    // Constructor
    InstancedMeshGroup() {
        meshData = std::make_unique<RawMeshData>();
        materialDef = std::make_unique<MaterialDefinition>();
    }

    // Move semantics
    InstancedMeshGroup(InstancedMeshGroup&&) = default;
    InstancedMeshGroup& operator=(InstancedMeshGroup&&) = default;

    // Delete copy
    InstancedMeshGroup(const InstancedMeshGroup&) = delete;
    InstancedMeshGroup& operator=(const InstancedMeshGroup&) = delete;
};
