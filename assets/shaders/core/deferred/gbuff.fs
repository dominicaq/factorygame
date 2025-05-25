#version 460 core
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : enable

// G-buffer outputs
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gPBRParams;
layout (location = 4) out vec3 gEmissive;

// Input from vertex shader
in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;
in vec3 TangentViewPos;
in vec3 TangentFragPos;
flat in uint MaterialID;

// Material data structure (same as your C++ struct)
struct MaterialData {
    sampler2D albedoMapSampler;
    sampler2D normalMapSampler;
    sampler2D metallicRoughnessMapSampler;
    sampler2D emissiveMapSampler;
    sampler2D heightMapSampler;

    uint64_t _padding0;

    vec4 albedoColor;
    vec4 emissiveColor;
    vec2 uvScale;

    uint64_t _padding1;

    float heightScale;
    float occlusionStrength;
    float shininess;
    float time;

    uint textureFlags;
    uint _padding2[3];
};

// Material buffer
layout (std430, binding = 1) buffer MaterialBuffer {
    MaterialData materials[];
};

// Material flags
const uint MATERIAL_HAS_ALBEDO_MAP = 1u;
const uint MATERIAL_HAS_NORMAL_MAP = 2u;
const uint MATERIAL_HAS_METALLIC_ROUGHNESS_MAP = 4u;
const uint MATERIAL_HAS_EMISSIVE_MAP = 8u;
const uint MATERIAL_HAS_HEIGHT_MAP = 16u;

// Helper function to check texture flags
bool hasTextureFlag(uint flags, uint flag) {
    return (flags & flag) != 0u;
}

// Apply tiling using material's uvScale
vec2 applyTiling(vec2 texCoords, vec2 uvScale) {
    return fract(texCoords * uvScale);
}

// Parallax mapping function
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir, sampler2D heightMap, float heightScale) {
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(normalize(vec3(0.0, 0.0, 1.0)), viewDir)));
    float layerDepth = 1.0 / numLayers;
    float curvatureFactor = clamp(dot(normalize(Normal), vec3(0.0, 0.0, 1.0)), 0.5, 1.0);
    float adjustedHeightScale = heightScale * curvatureFactor;
    float currentLayerDepth = 0.0;
    vec2 P = viewDir.xy / viewDir.z * adjustedHeightScale;
    vec2 deltaTexCoords = P / numLayers;
    vec2 currentTexCoords = texCoords;
    float currentDepthMapValue = texture(heightMap, currentTexCoords).r;

    while (currentLayerDepth < currentDepthMapValue) {
        currentTexCoords -= deltaTexCoords;
        currentDepthMapValue = texture(heightMap, currentTexCoords).r;
        currentLayerDepth += layerDepth;
    }

    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
    float afterDepth = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(heightMap, prevTexCoords).r - currentLayerDepth + layerDepth;
    float weight = afterDepth / (afterDepth - beforeDepth);
    return prevTexCoords * weight + currentTexCoords * (1.0 - weight);
}

void main() {
    // Get material data
    MaterialData material = materials[MaterialID];

    // Apply tiling using material's uvScale
    vec2 texCoords = applyTiling(TexCoords, material.uvScale);

    // Apply parallax mapping if height map is available
    if (hasTextureFlag(material.textureFlags, MATERIAL_HAS_HEIGHT_MAP)) {
        vec3 viewDir = normalize(TangentViewPos - TangentFragPos);
        texCoords = ParallaxMapping(texCoords, viewDir, material.heightMapSampler, material.heightScale);
        texCoords = fract(texCoords);
    }

    // Write position to G-buffer
    gPosition = FragPos;

    // Handle normal mapping
    vec3 mappedNormal = normalize(Normal);
    if (hasTextureFlag(material.textureFlags, MATERIAL_HAS_NORMAL_MAP)) {
        mat3 TBN = mat3(normalize(Tangent), normalize(Bitangent), normalize(Normal));
        vec3 normalSample = texture(material.normalMapSampler, texCoords).rgb;
        normalSample = normalSample * 2.0 - 1.0;
        mappedNormal = normalize(TBN * normalSample);
    }
    gNormal = mappedNormal;

    // Handle albedo
    vec3 albedo = material.albedoColor.rgb;
    if (hasTextureFlag(material.textureFlags, MATERIAL_HAS_ALBEDO_MAP)) {
        albedo *= texture(material.albedoMapSampler, texCoords).rgb;
    }
    gAlbedo.rgb = albedo;
    gAlbedo.a = material.albedoColor.a;

    // Handle PBR parameters (metallic, roughness, AO)
    float ao = material.occlusionStrength;
    float roughness = 1.0;
    float metallic = 0.0;

    if (hasTextureFlag(material.textureFlags, MATERIAL_HAS_METALLIC_ROUGHNESS_MAP)) {
        vec3 mr = texture(material.metallicRoughnessMapSampler, texCoords).rgb;
        ao *= mr.r;        // Occlusion
        roughness = mr.g;  // Roughness
        metallic = mr.b;   // Metallic
    }
    gPBRParams = vec4(metallic, roughness, ao, 1.0);

    // Handle emissive
    vec3 emissive = vec3(0.0);
    if (hasTextureFlag(material.textureFlags, MATERIAL_HAS_EMISSIVE_MAP)) {
        emissive = texture(material.emissiveMapSampler, texCoords).rgb * material.emissiveColor.rgb;
    }
    gEmissive = emissive;
}
