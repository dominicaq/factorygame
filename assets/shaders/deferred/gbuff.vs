#version 330 core

layout (location = 0) in vec3 aPos;        // Vertex position
layout (location = 1) in vec3 aNormal;     // Vertex normal
layout (location = 2) in vec2 aTexCoords;  // Texture coordinates

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main() {
    // Transform the position to world space
    FragPos = vec3(u_Model * vec4(aPos, 1.0));

    // Pass texture coordinates and normals to fragment shader
    TexCoords = aTexCoords;
    Normal = mat3(transpose(inverse(u_Model))) * aNormal;

    // Transform the vertex position to clip space
    gl_Position = u_Projection * u_View * vec4(FragPos, 1.0);
}
