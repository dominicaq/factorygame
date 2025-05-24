#pragma once

#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>
#include <iostream>

#include "material.h"

// Forward declaration to avoid circular includes
class Texture;

// Hash function for MaterialDefinition to enable deduplication
struct MaterialDefinitionHash {
    std::size_t operator()(const MaterialDefinition& def) const {
        std::size_t hash = 0;

        // Hash texture paths
        hash ^= std::hash<std::string>{}(def.albedoMapPath) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= std::hash<std::string>{}(def.normalMapPath) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= std::hash<std::string>{}(def.metallicRoughnessMapPath) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= std::hash<std::string>{}(def.emissiveMapPath) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= std::hash<std::string>{}(def.heightMapPath) + 0x9e3779b9 + (hash << 6) + (hash >> 2);

        // Hash material properties (convert floats to bits for consistent hashing)
        hash ^= std::hash<uint32_t>{}(*reinterpret_cast<const uint32_t*>(&def.heightScale));
        hash ^= std::hash<uint32_t>{}(*reinterpret_cast<const uint32_t*>(&def.occlusionStrength));
        hash ^= std::hash<uint32_t>{}(*reinterpret_cast<const uint32_t*>(&def.shininess));

        return hash;
    }
};

// Equality operator for MaterialDefinition
struct MaterialDefinitionEqual {
    bool operator()(const MaterialDefinition& lhs, const MaterialDefinition& rhs) const {
        return lhs.albedoMapPath == rhs.albedoMapPath &&
               lhs.normalMapPath == rhs.normalMapPath &&
               lhs.metallicRoughnessMapPath == rhs.metallicRoughnessMapPath &&
               lhs.emissiveMapPath == rhs.emissiveMapPath &&
               lhs.heightMapPath == rhs.heightMapPath &&
               lhs.albedoColor.x == rhs.albedoColor.x &&
               lhs.albedoColor.y == rhs.albedoColor.y &&
               lhs.albedoColor.z == rhs.albedoColor.z &&
               lhs.albedoColor.w == rhs.albedoColor.w &&
               lhs.emissiveColor.x == rhs.emissiveColor.x &&
               lhs.emissiveColor.y == rhs.emissiveColor.y &&
               lhs.emissiveColor.z == rhs.emissiveColor.z &&
               lhs.uvScale.x == rhs.uvScale.x &&
               lhs.uvScale.y == rhs.uvScale.y &&
               lhs.heightScale == rhs.heightScale &&
               lhs.occlusionStrength == rhs.occlusionStrength &&
               lhs.shininess == rhs.shininess;
    }
};

class MaterialManager {
public:
    static MaterialManager& getInstance() {
        static MaterialManager instance;
        return instance;
    }

    ~MaterialManager();

    // Initialize the material system
    bool initialize(size_t maxMaterials = 10000);
    void cleanup();

    // Entry point - converts MaterialDefinition to material index
    uint32_t getMaterialIndex(const MaterialDefinition& materialDef);

    // Update material SSBO on GPU
    void updateMaterialBuffer();

    // Bind material buffer for rendering
    void bindMaterialBuffer(GLuint bindingPoint = 0);

    // Debug info
    size_t getMaterialCount() const { return m_materials.size(); }
    size_t getTextureCount() const { return m_textureCache.size(); }
    void printStats() const;

private:
    MaterialManager() = default;
    MaterialManager(const MaterialManager&) = delete;
    MaterialManager& operator=(const MaterialManager&) = delete;

    // Load a texture by filepath and return its handle (with deduplication)
    GLuint64 getTextureHandle(const std::string& filePath);

    // Convert MaterialDefinition to MaterialData
    MaterialData createMaterialData(const MaterialDefinition& def);

    // GPU storage
    GLuint m_materialSSBO = 0;
    std::vector<MaterialData> m_materials;
    size_t m_maxMaterials = 0;

    // Material deduplication
    std::unordered_map<MaterialDefinition, uint32_t, MaterialDefinitionHash, MaterialDefinitionEqual> m_materialMap;

    // Texture deduplication - maps filepath to texture
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textureCache;

    bool m_initialized = false;
    bool m_needsUpdate = false;
};
