#pragma once

#include "../components/mesh.h"

// Generate primitive shapes
namespace MeshGen {
    Mesh* createCube();
    Mesh* createQuad(float scale);
    Mesh* createSphere(float radius, int sectors, int stacks);
    Mesh* createCapsule(float radius, float height, int sectors, int stacks);
}
