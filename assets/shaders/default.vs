#version 330 core

// Input attributes from renderer
layout (location = 0) in vec3 aPos;       // Vertex position
layout (location = 1) in vec3 aNormal;    // Vertex normal
layout (location = 2) in vec2 aTexCoord;  // Texture coordinates

// Output to fragment shader
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

// Uniforms for transforming the vertex position and normals
uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main() {
    // Transform the vertex position to world space
    FragPos = vec3(u_Model * vec4(aPos, 1.0));

    // Correctly transform the normal to world space (using transpose of the inverse of the model matrix)
    Normal = aNormal;

    // Pass texture coordinates unchanged
    TexCoord = aTexCoord;

    // Final vertex position to be passed to the fragment shader (full MVP transformation)
    gl_Position = u_Projection * u_View * u_Model * vec4(aPos, 1.0);
}
