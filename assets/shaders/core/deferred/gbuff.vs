#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec4 aPackedNormalTangent;

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

// Unpack normal from quaternion using Filament's method
vec3 unpackNormal(vec4 q) {
    // Constants from JavaScript: F0, F1, F2
    vec3 n0 = vec3(0.0, 0.0, 1.0);
    vec3 n1 = vec3(2.0, -2.0, -2.0) * q.x * vec3(q.z, q.w, q.x);
    vec3 n2 = vec3(2.0, 2.0, -2.0) * q.y * vec3(q.w, q.z, q.y);

    return n0 + n1 + n2;
}

// Unpack tangent from quaternion using Filament's method
vec3 unpackTangent(vec4 q) {
    // Constants from JavaScript: Q0, Q1, Q2
    vec3 t0 = vec3(1.0, 0.0, 0.0);
    vec3 t1 = vec3(-2.0, 2.0, -2.0) * q.y * vec3(q.y, q.x, q.w);
    vec3 t2 = vec3(-2.0, 2.0, 2.0) * q.z * vec3(q.z, q.w, q.x);

    return t0 + t1 + t2;
}

void main() {
    mat4 modelMatrix = (gl_InstanceID > 0) ? models[gl_InstanceID - 1] : u_Model;

    FragPos = vec3(modelMatrix * vec4(aPos, 1.0));
    TexCoords = aTexCoords * u_uvScale;

    // Normalize the packed quaternion
    vec4 q = normalize(aPackedNormalTangent);

    // Ensure consistent handedness
    if (q.w < 0.0) {
        q = -q;
    }

    // Unpack normal and tangent using Filament's formulas
    vec3 localNormal = unpackNormal(q);
    vec3 localTangent = unpackTangent(q);

    // Compute bitangent as cross product
    vec3 localBitangent = cross(localNormal, localTangent);

    // Transform to world space
    mat3 normalMatrix = mat3(modelMatrix);
    Normal = normalize(normalMatrix * localNormal);
    Tangent = normalize(normalMatrix * localTangent);
    Bitangent = normalize(normalMatrix * localBitangent);

    // Create TBN matrix for tangent space calculations
    mat3 TBN = mat3(Tangent, Bitangent, Normal);

    TangentViewPos = TBN * u_ViewPos;
    TangentFragPos = TBN * FragPos;

    gl_Position = u_Projection * u_View * vec4(FragPos, 1.0);
}
