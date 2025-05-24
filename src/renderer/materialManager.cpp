#include "materialManager.h"
#include "texture.h"

MaterialManager::~MaterialManager() {
    cleanup();
}

bool MaterialManager::initialize(size_t maxMaterials) {
    if (m_initialized) {
        return true;
    }

    // Check if bindless textures are supported
    if (!glGetTextureHandleARB) {
        std::cerr << "[Error] MaterialManager: Bindless textures not supported\n";
        return false;
    }

    m_maxMaterials = maxMaterials;
    m_materials.reserve(maxMaterials);

    // Create SSBO for material data
    glGenBuffers(1, &m_materialSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_materialSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 maxMaterials * sizeof(MaterialData),
                 nullptr,
                 GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    m_initialized = true;
    std::cout << "[Info] MaterialManager: Initialized with max " << maxMaterials << " materials\n";
    return true;
}

void MaterialManager::cleanup() {
    // Clean up textures
    m_textureCache.clear();

    if (m_materialSSBO != 0) {
        glDeleteBuffers(1, &m_materialSSBO);
        m_materialSSBO = 0;
    }

    m_materials.clear();
    m_materialMap.clear();
    m_initialized = false;
}

GLuint64 MaterialManager::getTextureHandle(const std::string& filePath) {
    if (filePath.empty()) {
        return 0;
    }

    // Check if texture already exists
    auto it = m_textureCache.find(filePath);
    if (it != m_textureCache.end()) {
        // Texture already loaded, return existing handle
        return it->second->getHandle();
    }

    // Load new texture
    auto texture = std::make_shared<Texture>(filePath);
    if (texture->isValid()) {
        texture->makeResident();
        m_textureCache[filePath] = texture;
        // std::cout << "[Info] MaterialManager: Loaded texture: " << filePath << "\n";
        return texture->getHandle();
    } else {
        std::cerr << "[Error] MaterialManager: Failed to load texture: " << filePath << "\n";
        return 0;
    }
}

MaterialData MaterialManager::createMaterialData(const MaterialDefinition& def) {
    MaterialData data;

    // Copy material properties
    data.albedoColor = def.albedoColor;
    data.emissiveColor = def.emissiveColor;
    data.uvScale = def.uvScale;
    data.heightScale = def.heightScale;
    data.occlusionStrength = def.occlusionStrength;
    data.shininess = def.shininess;
    data.time = def.time;

    // Load textures and set handles + flags
    GLuint64 handle = getTextureHandle(def.albedoMapPath);
    if (handle != 0) {
        data.albedoMapHandle = MaterialData::splitHandle(handle);
        data.setTextureFlag(MATERIAL_HAS_ALBEDO_MAP);
    }

    handle = getTextureHandle(def.normalMapPath);
    if (handle != 0) {
        data.normalMapHandle = MaterialData::splitHandle(handle);
        data.setTextureFlag(MATERIAL_HAS_NORMAL_MAP);
    }

    handle = getTextureHandle(def.metallicRoughnessMapPath);
    if (handle != 0) {
        data.metallicRoughnessMapHandle = MaterialData::splitHandle(handle);
        data.setTextureFlag(MATERIAL_HAS_METALLIC_ROUGHNESS_MAP);
    }

    handle = getTextureHandle(def.emissiveMapPath);
    if (handle != 0) {
        data.emissiveMapHandle = MaterialData::splitHandle(handle);
        data.setTextureFlag(MATERIAL_HAS_EMISSIVE_MAP);
    }

    handle = getTextureHandle(def.heightMapPath);
    if (handle != 0) {
        data.heightMapHandle = MaterialData::splitHandle(handle);
        data.setTextureFlag(MATERIAL_HAS_HEIGHT_MAP);
    }

    return data;
}

uint32_t MaterialManager::getMaterialIndex(const MaterialDefinition& materialDef) {
    if (!m_initialized) {
        std::cerr << "[Error] MaterialManager: Not initialized\n";
        return 0;
    }

    // Check if this exact material definition already exists
    auto it = m_materialMap.find(materialDef);
    if (it != m_materialMap.end()) {
        // Return existing material index
        std::cout << "[Info] MaterialManager: Reusing existing material " << it->second << "\n";
        return it->second;
    }

    // Create new material
    if (m_materials.size() >= m_maxMaterials) {
        std::cerr << "[Error] MaterialManager: Maximum materials exceeded\n";
        return 0;
    }

    uint32_t materialIndex = static_cast<uint32_t>(m_materials.size());
    MaterialData materialData = createMaterialData(materialDef);

    m_materials.push_back(materialData);
    m_materialMap[materialDef] = materialIndex;
    m_needsUpdate = true;

    // std::cout << "[Info] MaterialManager: Created new material " << materialIndex
    //           << " (Total: " << m_materials.size() << ")\n";

    return materialIndex;
}

void MaterialManager::updateMaterialBuffer() {
    if (!m_initialized || !m_needsUpdate) return;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_materialSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,
                   m_materials.size() * sizeof(MaterialData),
                   m_materials.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    m_needsUpdate = false;
    std::cout << "[Info] MaterialManager: Updated GPU buffer with " << m_materials.size() << " materials\n";
}

void MaterialManager::bindMaterialBuffer(GLuint bindingPoint) {
    if (!m_initialized) return;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, m_materialSSBO);
}

void MaterialManager::printStats() const {
    std::cout << "[Stats] MaterialManager:\n";
    std::cout << "  Materials: " << m_materials.size() << " / " << m_maxMaterials << "\n";
    std::cout << "  Unique Textures: " << m_textureCache.size() << "\n";

    // Count texture usage
    std::unordered_map<std::string, int> textureUsage;
    for (const auto& [def, index] : m_materialMap) {
        if (!def.albedoMapPath.empty()) textureUsage[def.albedoMapPath]++;
        if (!def.normalMapPath.empty()) textureUsage[def.normalMapPath]++;
        if (!def.metallicRoughnessMapPath.empty()) textureUsage[def.metallicRoughnessMapPath]++;
        if (!def.emissiveMapPath.empty()) textureUsage[def.emissiveMapPath]++;
        if (!def.heightMapPath.empty()) textureUsage[def.heightMapPath]++;
    }

    int totalTextureReferences = 0;
    for (const auto& [path, count] : textureUsage) {
        totalTextureReferences += count;
    }

    std::cout << "  Total Texture References: " << totalTextureReferences << "\n";
    std::cout << "  Deduplication Savings: " << (totalTextureReferences - m_textureCache.size()) << " textures\n";
}
