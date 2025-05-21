#version 330 core

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

// PBR Materials
uniform vec4 u_AlbedoColor = vec4(1.0);
uniform bool u_HasNormalMap = false;
uniform bool u_HasMetallicRoughnessMap = false;
uniform bool u_HasHeightMap = false;
uniform bool u_HasEmissiveMap = false;

uniform sampler2D u_AlbedoMap;
uniform sampler2D u_NormalMap;
uniform sampler2D u_MetallicRoughnessMap;
uniform sampler2D u_HeightMap;
uniform sampler2D u_EmissiveMap;

uniform vec2 u_uvScale = vec2(1.0, 1.0);
uniform float u_HeightScale = 0.05;
uniform vec3 u_EmissiveColor = vec3(1.0);
uniform float u_EmissiveIntensity = 1.0;

vec2 applyTiling(vec2 texCoords) {
    return fract(texCoords * u_uvScale);
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir) {
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(normalize(vec3(0.0, 0.0, 1.0)), viewDir)));
    float layerDepth = 1.0 / numLayers;

    float curvatureFactor = clamp(dot(normalize(Normal), vec3(0.0, 0.0, 1.0)), 0.5, 1.0);
    float adjustedHeightScale = u_HeightScale * curvatureFactor;

    float currentLayerDepth = 0.0;
    vec2 P = viewDir.xy / viewDir.z * adjustedHeightScale;
    vec2 deltaTexCoords = P / numLayers;

    vec2 currentTexCoords = texCoords;
    float currentDepthMapValue = texture(u_HeightMap, currentTexCoords).r;

    // Flip heightmap interpretation
    while (currentLayerDepth < (1.0 - currentDepthMapValue)) {
        currentTexCoords -= deltaTexCoords;
        currentDepthMapValue = texture(u_HeightMap, currentTexCoords).r;
        currentLayerDepth += layerDepth;
    }

    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
    float afterDepth = (1.0 - currentDepthMapValue) - currentLayerDepth;
    float beforeDepth = (1.0 - texture(u_HeightMap, prevTexCoords).r) - currentLayerDepth + layerDepth;
    float weight = afterDepth / (afterDepth - beforeDepth);
    return prevTexCoords * weight + currentTexCoords * (1.0 - weight);
}

void main() {
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
        vec3 normalSample = texture(u_NormalMap, texCoords).rgb;
        normalSample = normalSample * 2.0 - 1.0;
        mappedNormal = normalize(TBN * normalSample);
    }
    gNormal = mappedNormal;

    vec3 albedo = texture(u_AlbedoMap, texCoords).rgb * u_AlbedoColor.rgb;
    gAlbedo.rgb = albedo;
    gAlbedo.a = u_AlbedoColor.a;

    float ao = 1.0;
    float roughness = 1.0;
    float metallic = 0.0;

    if (u_HasMetallicRoughnessMap) {
        vec3 mr = texture(u_MetallicRoughnessMap, texCoords).rgb;
        ao = mr.r;
        roughness = mr.g;
        metallic = mr.b;
    }

    gPBRParams = vec4(metallic, roughness, ao, 1.0);

    vec3 emissive = vec3(0.0);
    if (u_HasEmissiveMap) {
        emissive = texture(u_EmissiveMap, texCoords).rgb * u_EmissiveColor * u_EmissiveIntensity;
    }
    gEmissive = emissive;
}
