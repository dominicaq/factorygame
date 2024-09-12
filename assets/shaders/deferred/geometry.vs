#version 330 core

// Input vertex attributes from the mesh
layout (location = 0) in vec3 aPos;       // Vertex position
layout (location = 1) in vec3 aNormal;    // Vertex normal
layout (location = 2) in vec2 aTexCoords; // Texture coordinates

// Outputs to the fragment shader
out vec3 WorldPos;    // World-space position
out vec3 NormalDir;   // World-space normal
out vec2 TexCoords;   // Texture coordinates

// Uniforms for transformations
uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
    // Compute world-space position of the vertex
    WorldPos = vec3(u_Model * vec4(aPos, 1.0));

    // Transform the normal using the model matrix (without translation)
    NormalDir = mat3(transpose(inverse(u_Model))) * aNormal;

    // Pass the texture coordinates through
    TexCoords = aTexCoords;

    // Compute the final clip-space position of the vertex
    gl_Position = u_Projection * u_View * vec4(WorldPos, 1.0);
}
