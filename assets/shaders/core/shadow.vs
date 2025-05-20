#version 430 core

layout (location = 0) in vec3 aPos;

layout (std430, binding = 0) buffer InstanceMatrices {
    mat4 models[];
};

uniform mat4 u_Model;
uniform mat4 u_LightSpaceMatrix;

void main() {
    mat4 modelMatrix = (gl_InstanceID > 0) ? models[gl_InstanceID - 1] : u_Model;
    gl_Position = u_LightSpaceMatrix * modelMatrix * vec4(aPos, 1.0);
}
