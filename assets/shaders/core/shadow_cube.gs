#version 410 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

uniform mat4 u_LightSpaceMatrices[6]; // Array of light-space matrices for each cubemap face

void main() {
    for (int face = 0; face < 6; ++face) {
        for (int i = 0; i < 3; ++i) {
            gl_Position = u_LightSpaceMatrices[face] * gl_in[i].gl_Position;
            EmitVertex();
        }
        EndPrimitive();
    }
}
