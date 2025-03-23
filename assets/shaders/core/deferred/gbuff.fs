#version 330 core

// G-buffer outputs
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gPBRParams;

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;
// Parallax Mapping
in vec3 TangentViewPos;
in vec3 TangentFragPos;

// PBR Materials
uniform vec4 u_AlbedoColor;
uniform sampler2D u_AlbedoMap;

uniform bool u_HasNormalMap;
uniform bool u_HasMetallicMap;
uniform bool u_HasRoughnessMap;
uniform bool u_HasAOMap;
uniform bool u_HasHeightMap;

uniform sampler2D u_NormalMap;
uniform sampler2D u_MetallicMap;
uniform sampler2D u_RoughnessMap;
uniform sampler2D u_AOMap;
uniform sampler2D u_HeightMap;

uniform float u_HeightScale;
uniform vec2 u_uvScale;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
    float layerDepth = 1.0 / numLayers;

    float curvatureFactor = clamp(dot(Normal, vec3(0.0, 0.0, 1.0)), 0.5, 1.0);
    float adjustedHeightScale = u_HeightScale * curvatureFactor;

    float currentLayerDepth = 0.0;
    vec2 P = viewDir.xy / viewDir.z * adjustedHeightScale;
    vec2 deltaTexCoords = P / numLayers;

    vec2 currentTexCoords = texCoords;
    float currentDepthMapValue = texture(u_HeightMap, currentTexCoords).r;

    while (currentLayerDepth < currentDepthMapValue)
    {
        currentTexCoords -= deltaTexCoords;
        currentDepthMapValue = texture(u_HeightMap, currentTexCoords).r;
        currentLayerDepth += layerDepth;
    }

    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
    float afterDepth = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(u_HeightMap, prevTexCoords).r - currentLayerDepth + layerDepth;
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

// Apply tiling to texture coordinates
vec2 applyTiling(vec2 texCoords) {
    vec2 scaledCoords = texCoords * u_uvScale;
    return fract(scaledCoords);
}

void main() {
    // Apply scaling and tiling to base texture coordinates
    vec2 texCoords = applyTiling(TexCoords);

    if (u_HasHeightMap) {
        vec3 viewDir = normalize(TangentViewPos - TangentFragPos);
        texCoords = ParallaxMapping(texCoords, viewDir);
        texCoords = fract(texCoords);
    }

    gPosition = FragPos;
    vec3 mappedNormal = normalize(Normal);

    if (u_HasNormalMap) {
        mat3 TBN = mat3(normalize(Tangent), normalize(Bitangent), normalize(Normal));
        vec3 normalMap = texture(u_NormalMap, texCoords).rgb;
        normalMap = normalMap * 2.0 - 1.0;
        mappedNormal = normalize(TBN * normalMap);
    }

    gNormal = mappedNormal;

    vec3 albedo = texture(u_AlbedoMap, texCoords).rgb * u_AlbedoColor.rgb;
    gAlbedo.rgb = albedo;
    gAlbedo.a = 1.0;

    float metallic = u_HasMetallicMap ? texture(u_MetallicMap, texCoords).r : 0.0;
    float roughness = u_HasRoughnessMap ? texture(u_RoughnessMap, texCoords).r : 1.0;
    float ao = u_HasAOMap ? texture(u_AOMap, texCoords).r : 1.0;

    gPBRParams = vec4(metallic, roughness, ao, 1.0);
}
