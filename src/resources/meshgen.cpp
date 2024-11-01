#include "meshgen.h"

#include <glm/glm.hpp>

Mesh* MeshGen::createCube() {
    Mesh* cubeMesh = new Mesh();

    // Positions for the cube
    cubeMesh->vertices = {
        { -0.5f, -0.5f, -0.5f }, { 0.5f,  0.5f, -0.5f }, { 0.5f, -0.5f, -0.5f }, { 0.5f,  0.5f, -0.5f },
        { -0.5f, -0.5f, -0.5f }, { -0.5f,  0.5f, -0.5f }, { -0.5f, -0.5f,  0.5f }, { 0.5f, -0.5f,  0.5f },
        { 0.5f,  0.5f,  0.5f }, { 0.5f,  0.5f,  0.5f }, { -0.5f,  0.5f,  0.5f }, { -0.5f, -0.5f,  0.5f },
        { -0.5f,  0.5f,  0.5f }, { -0.5f,  0.5f, -0.5f }, { -0.5f, -0.5f, -0.5f }, { -0.5f, -0.5f, -0.5f },
        { -0.5f, -0.5f,  0.5f }, { -0.5f,  0.5f,  0.5f }, { 0.5f,  0.5f,  0.5f }, { 0.5f, -0.5f, -0.5f },
        { 0.5f,  0.5f, -0.5f }, { 0.5f, -0.5f, -0.5f }, { 0.5f,  0.5f,  0.5f }, { 0.5f, -0.5f,  0.5f },
        { -0.5f, -0.5f, -0.5f }, { 0.5f, -0.5f, -0.5f }, { 0.5f, -0.5f,  0.5f }, { 0.5f, -0.5f,  0.5f },
        { -0.5f, -0.5f,  0.5f }, { -0.5f, -0.5f, -0.5f }, { -0.5f,  0.5f, -0.5f }, { 0.5f,  0.5f,  0.5f },
        { 0.5f,  0.5f, -0.5f }, { 0.5f,  0.5f,  0.5f }, { -0.5f,  0.5f, -0.5f }, { -0.5f,  0.5f,  0.5f }
    };

    // Indices for the cube
    cubeMesh->indices = {
        0, 1, 2, 3, 4, 5,    // Front face
        6, 7, 8, 9, 10, 11,  // Back face
        12, 13, 14, 15, 16, 17, // Left face
        18, 19, 20, 21, 22, 23, // Right face
        24, 25, 26, 27, 28, 29, // Bottom face
        30, 31, 32, 33, 34, 35  // Top face
    };

    return cubeMesh;
}

Mesh* MeshGen::createQuad(float scale = 1.0f) {
    Mesh* quadMesh = new Mesh();

    // Positions for the quad (a flat square in the XY plane, scaled)
    quadMesh->vertices = {
        { -0.5f * scale, -0.5f * scale, 0.0f }, // Bottom-left
        {  0.5f * scale, -0.5f * scale, 0.0f }, // Bottom-right
        {  0.5f * scale,  0.5f * scale, 0.0f }, // Top-right
        { -0.5f * scale,  0.5f * scale, 0.0f }  // Top-left
    };

    // UVs for the quad (optional, used for texture mapping)
    quadMesh->uvs = {
        { 0.0f, 0.0f }, // Bottom-left
        { 1.0f, 0.0f }, // Bottom-right
        { 1.0f, 1.0f }, // Top-right
        { 0.0f, 1.0f }  // Top-left
    };

    // Indices for the quad (two triangles)
    quadMesh->indices = {
        0, 1, 2, // First triangle
        2, 3, 0  // Second triangle
    };

    return quadMesh;
}

Mesh* MeshGen::createSphere(float radius, int sectors, int stacks) {
    Mesh* sphere = new Mesh();

    return sphere;
}

Mesh* MeshGen::createCapsule(float radius, float height, int sectors, int stacks) {
    Mesh* capsule = new Mesh();

    // Combine sphere and cylinder logic

    return capsule;
}

const float* MeshGen::createCubeMapVerts() {
    static const float cubeMapVerts[] = {
        // positions
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    return cubeMapVerts;
}
