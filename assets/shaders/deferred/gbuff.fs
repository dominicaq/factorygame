#version 330 core

layout (location = 0) out vec3 gPosition;   // World position
layout (location = 1) out vec3 gNormal;     // Normal
layout (location = 2) out vec4 gAlbedo;     // Albedo (base color)

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal;

uniform sampler2D u_AlbedoTexture;
uniform vec3 u_AlbedoColor;

void main() {
    // Store the fragment position in the first G-buffer texture
    gPosition = FragPos;

    // Store the per-fragment normals in the second G-buffer texture
    gNormal = normalize(Normal);

    // Sample the albedo color from the texture, modulated by the albedo color uniform
    vec3 albedo = texture(u_AlbedoTexture, TexCoords).rgb * u_AlbedoColor;

    // Set the albedo (base color) in the G-buffer, alpha set to 1.0 for now
    gAlbedo.rgb = albedo;
    gAlbedo.a = 1.0;
}
