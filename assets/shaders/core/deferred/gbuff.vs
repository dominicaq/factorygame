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

void main() {
    mat4 modelMatrix = (gl_InstanceID > 0) ? models[gl_InstanceID - 1] : u_Model;

    FragPos = vec3(modelMatrix * vec4(aPos, 1.0));
    TexCoords = aTexCoords * u_uvScale;

    // Unpack the quaternion-encoded normal and tangent inline
    float handedness = sign(aPackedNormalTangent.w);
    vec4 q = vec4(aPackedNormalTangent.xyz, abs(aPackedNormalTangent.w));

    // Normalize quaternion to ensure it's unit length
    q = normalize(q);

    // Convert quaternion to rotation matrix and extract basis vectors
    float xx = q.x * q.x;
    float yy = q.y * q.y;
    float zz = q.z * q.z;
    float xy = q.x * q.y;
    float xz = q.x * q.z;
    float yz = q.y * q.z;
    float wx = q.w * q.x;
    float wy = q.w * q.y;
    float wz = q.w * q.z;

    // Extract tangent (first column of rotation matrix)
    vec3 localTangent = vec3(
        1.0 - 2.0 * (yy + zz),
        2.0 * (xy + wz),
        2.0 * (xz - wy)
    );

    // Extract bitangent (second column of rotation matrix)
    vec3 localBitangent = vec3(
        2.0 * (xy - wz),
        1.0 - 2.0 * (xx + zz),
        2.0 * (yz + wx)
    );

    // Extract normal (third column of rotation matrix)
    vec3 localNormal = vec3(
        2.0 * (xz + wy),
        2.0 * (yz - wx),
        1.0 - 2.0 * (xx + yy)
    );

    // Apply handedness to bitangent
    if (handedness < 0.0) {
        localBitangent = -localBitangent;
    }

    mat3 normalMatrix = mat3(modelMatrix);
    Normal = normalize(normalMatrix * localNormal);
    Tangent = normalize(normalMatrix * localTangent);
    Bitangent = normalize(normalMatrix * localBitangent);

    // Create TBN matrix for tangent space calculations
    mat3 TBN = mat3(Tangent, Bitangent, Normal);

    // Compute the view position in tangent space
    TangentViewPos = TBN * u_ViewPos;
    TangentFragPos = TBN * FragPos;

    gl_Position = u_Projection * u_View * vec4(FragPos, 1.0);
}
