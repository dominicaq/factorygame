#pragma once

#include "../components/mesh.h"

// Generate primitive shapes
namespace MeshGen {
    Mesh* createCube();
    Mesh* createQuad(float scale);
    Mesh* createSphere(unsigned int sectorCount, unsigned int stackCount);
    Mesh* createPlane(unsigned int resolutionX, unsigned int resolutionY, float width, float height);
    Mesh* createCapsule(float radius, float height, int sectors, int stacks);
    void computepackedTNBFrame(Mesh* mesh);
    const float* createCubeMapVerts();
    void packTBNframe(Mesh* mesh, const std::vector<glm::vec3>& normals,
             const std::vector<glm::vec4>& tangents);
}
