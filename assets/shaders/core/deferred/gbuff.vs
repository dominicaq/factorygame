#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

// Instance matrix block (SSBO)
layout (std430, binding = 0) buffer InstanceMatrices {
    mat4 models[];  // Dynamic array of matrices
};

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;
out vec3 Tangent;
out vec3 Bitangent;
out vec3 TangentFragPos;
out vec3 TangentViewPos;

uniform vec3 u_ViewPos;
uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform vec2 u_uvScale;

void main() {
    mat4 modelMatrix = (gl_InstanceID > 0) ? models[gl_InstanceID - 1] : u_Model;

    FragPos = vec3(modelMatrix * vec4(aPos, 1.0));
    TexCoords = aTexCoords * u_uvScale;

    Normal = normalize(mat3(modelMatrix) * aNormal);
    Tangent = normalize(mat3(modelMatrix) * aTangent);
    Bitangent = normalize(cross(Normal, Tangent));
    mat3 TBN = mat3(Tangent, Bitangent, Normal);

    // Compute the view position in world space
    TangentViewPos = TBN * u_ViewPos;
    TangentFragPos = TBN * FragPos;

    gl_Position = u_Projection * u_View * vec4(FragPos, 1.0);
}
