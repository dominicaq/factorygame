#version 330 core

layout (location = 0) out vec3 gPosition; // World position
layout (location = 1) out vec3 gNormal;   // Normal
layout (location = 2) out vec4 gAlbedoSpec; // Albedo (diffuse color)

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal;

uniform sampler2D texture_diffuse1;

void main() {
    // Store the fragment position in the first G-buffer texture
    gPosition = FragPos;

    // Store the per-fragment normals in the second G-buffer texture
    gNormal = normalize(Normal);

    // Sample the albedo color from a texture or set a default color
    gAlbedoSpec.rgb = texture(texture_diffuse1, TexCoords).rgb;

    // Optionally store specular intensity in the alpha channel (set to 1.0 for now)
    gAlbedoSpec.a = 1.0;
}
