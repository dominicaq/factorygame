#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

// Outputs to the fragment shader
out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;
out vec3 Tangent;
out vec3 Bitangent;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main() {
    // Transform the position to world space
    FragPos = vec3(u_Model * vec4(aPos, 1.0));

    // Pass texture coordinates to fragment shader
    TexCoords = aTexCoords;

    // Calculate the normal matrix (inverse transpose of the model matrix)
    mat3 normalMatrix = transpose(inverse(mat3(u_Model)));

    // Transform the normal, tangent, and bitangent to world space
    Normal = normalize(normalMatrix * aNormal);
    Tangent = normalize(normalMatrix * aTangent);
    Bitangent = normalize(normalMatrix * aBitangent);

    // Transform the vertex position to clip space
    gl_Position = u_Projection * u_View * vec4(FragPos, 1.0);
}
