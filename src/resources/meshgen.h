#pragma once

#include "../components/mesh.h"

// Generate primitive shapes
namespace MeshGen {
    RawMeshData* createCube();
    RawMeshData* createQuad(float scale);
    RawMeshData* createSphere(unsigned int sectorCount, unsigned int stackCount);
    RawMeshData* createPlane(unsigned int resolutionX, unsigned int resolutionY, float width, float height);
    RawMeshData* createCapsule(float radius, float height, int sectors, int stacks);
    void computepackedTNBFrame(RawMeshData* mesh);
    const float* createCubeMapVerts();
    void packTBNframe(RawMeshData* mesh, const std::vector<glm::vec3>& normals,
             const std::vector<glm::vec4>& tangents);
}
