#version 410 core

layout (location = 0) in vec3 aPos;

uniform mat4 u_Model;
uniform mat4 u_LightSpaceMatrix;

void main() {
    gl_Position = u_LightSpaceMatrix * u_Model * vec4(aPos, 1.0);
}
