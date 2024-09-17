#version 330 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;

uniform vec3 u_AlbedoColor;
uniform sampler2D u_AlbedoMap;

uniform bool u_HasNormalMap;
uniform sampler2D u_NormalMap;

// NOTE: Shader should only store data

void main() {
    // Store the fragment position in gPosition
    gPosition = FragPos;
    vec3 mappedNormal = normalize(Normal);

    // Apply normal mapping if available
    if (u_HasNormalMap) {
        mat3 TBN = mat3(normalize(Tangent), normalize(Bitangent), normalize(Normal));
        // Transform to world space using TBN matrix and map from [0,1] to [-1,1]
        vec3 normalMap = texture(u_NormalMap, TexCoords).rgb;
        normalMap = normalMap * 2.0 - 1.0;
        mappedNormal = normalize(TBN * normalMap);
    }

    // Store the world-space normal in gNormal
    gNormal = mappedNormal;

    // Store the albedo color in gAlbedo
    vec3 albedo = texture(u_AlbedoMap, TexCoords).rgb * u_AlbedoColor;
    gAlbedo.rgb = albedo;
    gAlbedo.a = 1.0;
}
