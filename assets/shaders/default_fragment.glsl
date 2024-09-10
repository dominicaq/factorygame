#version 330 core

// Output color of the fragment
out vec4 FragColor;

// Input from vertex shader
in vec3 FragPos;   // Position of the fragment in world space
in vec3 Normal;    // Normal vector from vertex shader
in vec2 TexCoord;  // Texture coordinates from vertex shader

// Uniforms
uniform sampler2D u_Texture;  // Texture sampler

void main() {
    // Sample the texture using the texture coordinates
    vec4 textureColor = texture(u_Texture, TexCoord);

    // Hardcoded object color (can be multiplied with texture color if needed)
    vec3 objectColor = vec3(1.0, 1.0, 1.0);

    // Combine texture color with object color (if needed)
    FragColor = textureColor;
}
