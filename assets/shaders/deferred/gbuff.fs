#version 330 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal;

uniform sampler2D u_AlbedoMap;
uniform sampler2D u_NormalMap;

uniform vec3 u_AlbedoColor;
uniform bool u_HasNormalMap;

void main() {
    // Store the fragment position in gPosition
    gPosition = FragPos;

    // Check if normal map is present
    if (u_HasNormalMap) {
        // Sample the normal from the normal map and remap from [0,1] to [-1,1]
        vec3 normalMap = texture(u_NormalMap, TexCoords).rgb;
        gNormal = normalize(normalMap * 2.0 - 1.0);  // Remap to [-1, 1] range
    } else {
        // Use the geometry normal if no normal map
        gNormal = normalize(Normal);
    }

    // Store the albedo color in gAlbedo
    vec3 albedo = texture(u_AlbedoMap, TexCoords).rgb * u_AlbedoColor;
    gAlbedo.rgb = albedo;
    gAlbedo.a = 1.0;
}
