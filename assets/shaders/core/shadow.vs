#version 410 core

layout (location = 0) in vec3 aPos;

// Instance matrix as 4 vec4s (you can't use mat4 directly as an attribute)
layout (location = 5) in vec4 instanceMatrix0;
layout (location = 6) in vec4 instanceMatrix1;
layout (location = 7) in vec4 instanceMatrix2;
layout (location = 8) in vec4 instanceMatrix3;

uniform mat4 u_Model;
uniform mat4 u_LightSpaceMatrix;

void main() {
    mat4 instanceMat = mat4(
        instanceMatrix0,
        instanceMatrix1,
        instanceMatrix2,
        instanceMatrix3
    );

    mat4 modelMatrix = (gl_InstanceID > 0) ? instanceMat : u_Model;
    gl_Position = u_LightSpaceMatrix * modelMatrix * vec4(aPos, 1.0);
}
