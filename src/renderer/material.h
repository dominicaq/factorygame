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
    uint64_t albedoMapHandle = 0;           // 0-7
    uint64_t normalMapHandle = 0;           // 8-15
    uint64_t metallicRoughnessMapHandle = 0; // 16-23
    uint64_t emissiveMapHandle = 0;         // 24-31
    uint64_t heightMapHandle = 0;           // 32-39

    uint64_t _padding0 = 0;                 // 40-47 (NSight shows data starts at 48)

    glm::vec4 albedoColor = glm::vec4(1.0f);    // 48-63
    glm::vec4 emissiveColor = glm::vec4(1.0f);  // 64-79
    glm::vec2 uvScale = glm::vec2(1.0f);        // 80-87

    uint64_t _padding1 = 0;                 // 88-95 (align floats to 16-byte boundary)

    float heightScale = 1.0f;               // 96-99
    float occlusionStrength = 1.0f;         // 100-103
    float shininess = 32.0f;                // 104-107
    float time = 0.0f;                      // 108-111

    uint32_t textureFlags = 0;              // 112-115
    uint32_t _padding2[3] = {0, 0, 0};      // 116-127

    // Helper functions remain the same
    void setTextureFlag(uint32_t flag) { textureFlags |= flag; }
    void clearTextureFlag(uint32_t flag) { textureFlags &= ~flag; }
    bool hasTextureFlag(uint32_t flag) const { return (textureFlags & flag) != 0; }
};
