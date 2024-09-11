#include "meshgen.h"

#include <glm.hpp>

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

Mesh* MeshGen::createSphere(float radius, int sectors, int stacks) {
    Mesh* sphere = new Mesh();

    return sphere;
}

Mesh* MeshGen::createCapsule(float radius, float height, int sectors, int stacks) {
    Mesh* capsule = new Mesh();

    // Combine sphere and cylinder logic

    return capsule;
}
