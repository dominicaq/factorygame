#pragma once

#include "mesh.h"

// Generate primitive shapes
namespace MeshGen {
    Mesh* createCube();
    Mesh* createSphere(float radius, int sectors, int stacks);
    Mesh* createCapsule(float radius, float height, int sectors, int stacks);
}
