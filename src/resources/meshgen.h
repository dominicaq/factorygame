#ifndef MESHGEN_H
#define MESHGEN_H

#include "../components/mesh.h"

// Generate primitive shapes
namespace MeshGen {
    Mesh* createCube();
    Mesh* createQuad(float scale);
    Mesh* createSphere(float radius, int sectors, int stacks);
    Mesh* createCapsule(float radius, float height, int sectors, int stacks);

    const float* createCubeMapVerts();
}

#endif // MESHGEN_H
