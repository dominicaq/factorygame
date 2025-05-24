#version 450 core
#extension GL_ARB_bindless_texture : require

// G-buffer outputs
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gPBRParams;
layout (location = 4) out vec3 gEmissive;

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;
in vec3 TangentViewPos;
in vec3 TangentFragPos;
flat in uint MaterialID;

// Material data structure - using uvec2 handles
struct MaterialData {
    // Texture handles (64-bit split into 2x32-bit)
    uvec2 albedoMapHandle;
    uvec2 normalMapHandle;
    uvec2 metallicRoughnessMapHandle;
    uvec2 emissiveMapHandle;
    uvec2 heightMapHandle;

    // Material properties
    vec4 albedoColor;
    vec3 emissiveColor;
    vec2 uvScale;

    float heightScale;
    float occlusionStrength;
    float shininess;
    float time;

    uint textureFlags;
    uint padding[3];
};

// Material buffer
layout(std430, binding = 1) readonly buffer MaterialBuffer {
    MaterialData materials[];
};

// Texture flag constants
const uint MATERIAL_HAS_ALBEDO_MAP = 1u << 0u;
const uint MATERIAL_HAS_NORMAL_MAP = 1u << 1u;
const uint MATERIAL_HAS_METALLIC_ROUGHNESS_MAP = 1u << 2u;
const uint MATERIAL_HAS_EMISSIVE_MAP = 1u << 3u;
const uint MATERIAL_HAS_HEIGHT_MAP = 1u << 4u;

// Helper function to check if a texture flag is set
bool hasTexture(uint flags, uint flag) {
    return (flags & flag) != 0u;
}

// Helper to check if handle is valid
bool isValidHandle(uvec2 handle) {
    return handle.x != 0u || handle.y != 0u;
}

vec2 applyTiling(vec2 texCoords, vec2 uvScale) {
    return fract(texCoords * uvScale);
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir, uvec2 heightHandle, float heightScale) {
    if (!isValidHandle(heightHandle)) {
        return texCoords;
    }

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
    float currentDepthMapValue = texture(sampler2D(heightHandle), currentTexCoords).r;

    while (currentLayerDepth < currentDepthMapValue) {
        currentTexCoords -= deltaTexCoords;
        currentDepthMapValue = texture(sampler2D(heightHandle), currentTexCoords).r;
        currentLayerDepth += layerDepth;
    }

    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
    float afterDepth = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(sampler2D(heightHandle), prevTexCoords).r - currentLayerDepth + layerDepth;
    float weight = afterDepth / (afterDepth - beforeDepth);
    return prevTexCoords * weight + currentTexCoords * (1.0 - weight);
}

void main() {
    // Get material data for this fragment
    MaterialData mat = materials[MaterialID];

    // Apply UV tiling using material's UV scale
    vec2 texCoords = applyTiling(TexCoords, mat.uvScale);

    // Parallax mapping if height map is present
    if (hasTexture(mat.textureFlags, MATERIAL_HAS_HEIGHT_MAP)) {
        vec3 viewDir = normalize(TangentViewPos - TangentFragPos);
        texCoords = ParallaxMapping(texCoords, viewDir, mat.heightMapHandle, mat.heightScale);
        texCoords = fract(texCoords);
    }

    gPosition = FragPos;

    // Normal mapping
    vec3 mappedNormal = normalize(Normal);
    if (hasTexture(mat.textureFlags, MATERIAL_HAS_NORMAL_MAP) && isValidHandle(mat.normalMapHandle)) {
        mat3 TBN = mat3(normalize(Tangent), normalize(Bitangent), normalize(Normal));
        vec3 normalSample = texture(sampler2D(mat.normalMapHandle), texCoords).rgb;
        normalSample = normalSample * 2.0 - 1.0;
        mappedNormal = normalize(TBN * normalSample);
    }
    gNormal = mappedNormal;

    // Albedo
    vec3 albedo = mat.albedoColor.rgb;
    if (hasTexture(mat.textureFlags, MATERIAL_HAS_ALBEDO_MAP) && isValidHandle(mat.albedoMapHandle)) {
        albedo *= texture(sampler2D(mat.albedoMapHandle), texCoords).rgb;
    }
    gAlbedo.rgb = albedo;
    gAlbedo.a = mat.albedoColor.a;

    // PBR parameters
    float ao = 1.0;
    float roughness = 1.0;
    float metallic = 0.0;

    if (hasTexture(mat.textureFlags, MATERIAL_HAS_METALLIC_ROUGHNESS_MAP) && isValidHandle(mat.metallicRoughnessMapHandle)) {
        vec3 mr = texture(sampler2D(mat.metallicRoughnessMapHandle), texCoords).rgb;
        ao = mr.r;
        roughness = mr.g;
        metallic = mr.b;
    }

    gPBRParams = vec4(metallic, roughness, ao, 1.0);

    // Emissive
    vec3 emissive = vec3(0.0);
    if (hasTexture(mat.textureFlags, MATERIAL_HAS_EMISSIVE_MAP) && isValidHandle(mat.emissiveMapHandle)) {
        emissive = texture(sampler2D(mat.emissiveMapHandle), texCoords).rgb * mat.emissiveColor;
    }
    gEmissive = emissive;
}
