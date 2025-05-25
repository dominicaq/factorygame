#version 460 core

layout (location = 0) in vec3 aPos;

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

uniform mat4 u_LightSpaceMatrix;

void main() {
    // Get instance data directly from SSBO
    uint instanceId = gl_BaseInstance + gl_InstanceID;
    InstanceData instance = instances[instanceId];
    mat4 modelMatrix = instance.modelMatrix;

    // Transform vertex position
    vec3 worldPos = vec3(modelMatrix * vec4(aPos, 1.0));
    gl_Position = u_LightSpaceMatrix * vec4(worldPos, 1.0);
}
