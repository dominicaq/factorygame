#version 330 core

// G-buffer outputs with packed PBR parameters
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gPBRParams;

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;

// Required
uniform vec4 u_AlbedoColor;
uniform sampler2D u_AlbedoMap;

// Optional PBR textures
uniform bool u_HasNormalMap;
uniform bool u_HasMetallicMap;
uniform bool u_HasRoughnessMap;
uniform bool u_HasAOMap;

uniform sampler2D u_NormalMap;
uniform sampler2D u_MetallicMap;
uniform sampler2D u_RoughnessMap;
uniform sampler2D u_AOMap;

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
    vec3 albedo = texture(u_AlbedoMap, TexCoords).rgb * u_AlbedoColor.xyz;
    gAlbedo.rgb = albedo;
    gAlbedo.a = 1.0;

    // Fetch metallic, roughness, and AO from their respective textures if they are available
    float metallic = u_HasMetallicMap ? texture(u_MetallicMap, TexCoords).r : 0.0;
    float roughness = u_HasRoughnessMap ? texture(u_RoughnessMap, TexCoords).r : 1.0;
    float ao = u_HasAOMap ? texture(u_AOMap, TexCoords).r : 1.0;

    // Pack PBR parameters into a single vec4 output
    gPBRParams = vec4(metallic, roughness, ao, 1.0);
}
