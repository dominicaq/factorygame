#pragma once

#include "shader.h"
#include "texture.h"
#include <glm/glm.hpp>

// Texture presence bit flags - shared between C++ and shaders
#define MATERIAL_HAS_ALBEDO_MAP             (1u << 0)  // 0x01
#define MATERIAL_HAS_NORMAL_MAP             (1u << 1)  // 0x02
#define MATERIAL_HAS_METALLIC_ROUGHNESS_MAP (1u << 2)  // 0x04
#define MATERIAL_HAS_EMISSIVE_MAP           (1u << 3)  // 0x08
#define MATERIAL_HAS_HEIGHT_MAP             (1u << 4)  // 0x10

// Table to fill out in material init
struct MaterialDefinition {
    // Shader Paths
    std::string vertexShaderPath;
    std::string fragmentShaderPath;

    // Texture file paths
    std::string albedoMapPath;
    std::string normalMapPath;
    std::string metallicRoughnessMapPath;
    std::string emissiveMapPath;
    std::string heightMapPath;

    // Material properties
    glm::vec4 albedoColor = glm::vec4(1.0f);
    glm::vec3 emissiveColor = glm::vec3(1.0f);
    glm::vec2 uvScale = glm::vec2(1.0f);

    float heightScale = 1.0f;
    float occlusionStrength = 1.0f;
    float shininess = 32.0f;
    float time = 0.0f;

    bool isDeferred = true;
};

// GPU material data with texture handles
struct MaterialData {
    // Texture handles (64-bit)
    GLuint64 albedoMapHandle = 0;
    GLuint64 normalMapHandle = 0;
    GLuint64 metallicRoughnessMapHandle = 0;
    GLuint64 emissiveMapHandle = 0;
    GLuint64 heightMapHandle = 0;

    // Material properties
    glm::vec4 albedoColor = glm::vec4(1.0f);
    glm::vec3 emissiveColor = glm::vec3(1.0f);
    glm::vec2 uvScale = glm::vec2(1.0f);

    float heightScale = 1.0f;
    float occlusionStrength = 1.0f;
    float shininess = 32.0f;
    float time = 0.0f;

    // Flags for which textures are present
    uint32_t textureFlags = 0;
    uint32_t padding[3]; // Ensure proper alignment

    // Helper functions for bit manipulation
    void setTextureFlag(uint32_t flag) { textureFlags |= flag; }
    void clearTextureFlag(uint32_t flag) { textureFlags &= ~flag; }
    bool hasTextureFlag(uint32_t flag) const { return (textureFlags & flag) != 0; }
};
