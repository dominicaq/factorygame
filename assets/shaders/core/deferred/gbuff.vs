#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in vec4 instanceMatrix0;
layout (location = 6) in vec4 instanceMatrix1;
layout (location = 7) in vec4 instanceMatrix2;
layout (location = 8) in vec4 instanceMatrix3;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;
out vec3 Tangent;
out vec3 Bitangent;
out vec3 ViewPos;   // View position to pass to fragment shader
out vec3 TangentFragPos;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform vec2 u_uvScale;

void main() {
    mat4 instanceMat = mat4(
        instanceMatrix0,
        instanceMatrix1,
        instanceMatrix2,
        instanceMatrix3
    );

    mat4 modelMatrix = (gl_InstanceID > 0) ? instanceMat : u_Model;

    FragPos = vec3(modelMatrix * vec4(aPos, 1.0));
    TexCoords = aTexCoords * u_uvScale;

    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));

    Normal = normalize(normalMatrix * aNormal);
    Tangent = normalize(normalMatrix * aTangent);
    Bitangent = normalize(cross(Normal, Tangent));
    mat3 TBN = transpose(mat3(Tangent, Bitangent, Normal));

    // Compute the view position in world space
    vec3 viewPos = inverse(u_View)[3].xyz;
    ViewPos = viewPos;  // Pass the camera position in world space

    TangentFragPos = TBN * FragPos;

    gl_Position = u_Projection * u_View * vec4(FragPos, 1.0);
}
