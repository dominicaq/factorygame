#version 330 core

// Input vertex attributes from the vertex buffer
layout(location = 0) in vec3 aPos;    // Position attribute
layout(location = 1) in vec3 aNormal; // Normal attribute
layout(location = 2) in vec2 aTexCoord; // Texture coordinates (UVs)

// Uniforms
uniform mat4 u_MVP;

// Output data to the fragment shader
out vec3 FragPos;   // Position of the fragment in world space
out vec3 Normal;    // Normal of the fragment for lighting
out vec2 TexCoord;  // Pass texture coordinates to the fragment shader

void main() {
    // Pass the fragment position to the fragment shader
    FragPos = aPos;

    // Pass the normal vector to the fragment shader
    Normal = aNormal;

    // Pass the texture coordinates
    TexCoord = aTexCoord;

    // Apply the MVP matrix to the vertex position
    gl_Position = u_MVP * vec4(aPos, 1.0);
}
