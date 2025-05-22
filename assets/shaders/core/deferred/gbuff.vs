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

void unpackTBN(vec4 packedData, out vec3 tangent, out vec3 bitangent, out vec3 normal) {
    vec3 q = packedData.xyz;
    float qw = abs(packedData.w);
    float h = sign(packedData.w);

    // All computations in minimal steps
    float x2 = q.x + q.x, y2 = q.y + q.y, z2 = q.z + q.z;
    float xx = q.x * x2, yy = q.y * y2, zz = q.z * z2;
    float xy = q.x * y2, xz = q.x * z2, yz = q.y * z2;
    float wx = qw * x2, wy = qw * y2, wz = qw * z2;

    tangent.x = 1.0 - yy - zz;
    tangent.y = xy + wz;
    tangent.z = xz - wy;

    bitangent.x = (xy - wz) * h;
    bitangent.y = (1.0 - xx - zz) * h;
    bitangent.z = (yz + wx) * h;

    normal.x = xz + wy;
    normal.y = yz - wx;
    normal.z = 1.0 - xx - yy;
}

void main() {
    mat4 modelMatrix = (gl_InstanceID > 0) ? models[gl_InstanceID - 1] : u_Model;

    FragPos = vec3(modelMatrix * vec4(aPos, 1.0));
    TexCoords = aTexCoords * u_uvScale;

    // Unpack TBN using optimized function
    vec3 localTangent, localBitangent, localNormal;
    unpackTBN(aPackedNormalTangent, localTangent, localBitangent, localNormal);

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
