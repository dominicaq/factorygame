#version 460 core

// Vertex attributes
layout(location = 0) in vec4 aPosition;  // xyz = position, w = packed UVs
layout(location = 1) in vec4 aPackedTNB; // Packed tangent space quaternion

// Instance data structure
struct InstanceData {
    mat4 modelMatrix;
    vec2 uvScale;
    uint materialId;
    uint padding;
};

// Instance buffer with interleaved data
layout (std430, binding = 0) buffer InstanceBuffer {
    InstanceData instances[];
};

// Output to fragment shader
out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;
out vec3 Tangent;
out vec3 Bitangent;
out vec3 TangentFragPos;
out vec3 TangentViewPos;
flat out uint MaterialID;

// View uniforms (these stay as uniforms since they're global)
uniform vec3 u_ViewPos;
uniform mat4 u_View;
uniform mat4 u_Projection;

// Unpack normal from quaternion using Filament's method
vec3 unpackNormal(vec4 q) {
    vec3 n0 = vec3(0.0, 0.0, 1.0);
    vec3 n1 = vec3(2.0, -2.0, -2.0) * q.x * vec3(q.z, q.w, q.x);
    vec3 n2 = vec3(2.0, 2.0, -2.0) * q.y * vec3(q.w, q.z, q.y);
    return n0 + n1 + n2;
}

// Unpack tangent from quaternion using Filament's method
vec3 unpackTangent(vec4 q) {
    vec3 t0 = vec3(1.0, 0.0, 0.0);
    vec3 t1 = vec3(-2.0, 2.0, -2.0) * q.y * vec3(q.y, q.x, q.w);
    vec3 t2 = vec3(-2.0, 2.0, 2.0) * q.z * vec3(q.z, q.w, q.x);
    return t0 + t1 + t2;
}

void main() {
    // Get instance data directly from SSBO
    InstanceData instance = instances[gl_BaseInstance + gl_InstanceID];
    mat4 modelMatrix = instance.modelMatrix;
    vec2 currentUVScale = instance.uvScale;
    MaterialID = instance.materialId;

    // Extract vertex position
    vec3 position = aPosition.xyz;

    // Unpack UVs from the packed float in position.w
    uint packedUV = floatBitsToUint(aPosition.w);
    vec2 uv = vec2(
        float(packedUV & 0xFFFFu) / 65535.0,
        float(packedUV >> 16) / 65535.0
    );

    // Transform position to world space
    FragPos = vec3(modelMatrix * vec4(position, 1.0));

    // Apply UV scaling from instance data
    TexCoords = uv * currentUVScale;

    // Normalize the packed quaternion for tangent space
    vec4 q = normalize(aPackedTNB);

    // Ensure consistent handedness
    if (q.w < 0.0) {
        q = -q;
    }

    // Unpack normal and tangent from quaternion
    vec3 localNormal = unpackNormal(q);
    vec3 localTangent = unpackTangent(q);

    // Compute bitangent as cross product
    vec3 localBitangent = cross(localNormal, localTangent);

    // Transform tangent space vectors to world space
    mat3 normalMatrix = mat3(modelMatrix);
    Normal = normalize(normalMatrix * localNormal);
    Tangent = normalize(normalMatrix * localTangent);
    Bitangent = normalize(normalMatrix * localBitangent);

    // Create TBN matrix for tangent space lighting calculations
    mat3 TBN = mat3(Tangent, Bitangent, Normal);

    // Transform view position and fragment position to tangent space
    TangentViewPos = TBN * u_ViewPos;
    TangentFragPos = TBN * FragPos;

    // Final vertex position in clip space
    gl_Position = u_Projection * u_View * vec4(FragPos, 1.0);
}
