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

Mesh* MeshGen::createSphere(unsigned int sectorCount, unsigned int stackCount) {
    Mesh* sphere = new Mesh();

    const float radius = 1.0f;
    const float PI = 3.14159265358979323846f;

    // Clear any existing data
    sphere->vertices.clear();
    sphere->normals.clear();
    sphere->uvs.clear();
    sphere->indices.clear();

    float x, y, z, xy;                  // vertex position
    float nx, ny, nz;                   // vertex normal
    float s, t;                         // vertex texCoord

    float sectorStep = 2 * PI / sectorCount;
    float stackStep = PI / stackCount;
    float sectorAngle, stackAngle;

    // Compute all vertices first (for spheres, vertex position = normal)
    for(unsigned int i = 0; i <= stackCount; ++i) {
        stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);               // r * cos(u)
        z = radius * sinf(stackAngle);                // r * sin(u)

        // Add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for(unsigned int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep;             // starting from 0 to 2pi

            // vertex position
            x = xy * cosf(sectorAngle);               // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);               // r * cos(u) * sin(v)
            sphere->vertices.push_back(glm::vec3(x, y, z));

            // normalized vertex normal
            nx = x / radius;
            ny = y / radius;
            nz = z / radius;
            sphere->normals.push_back(glm::vec3(nx, ny, nz));

            // vertex tex coord between [0, 1]
            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            sphere->uvs.push_back(glm::vec2(s, t));
        }
    }

    // Indices
    unsigned int k1, k2;
    for(unsigned int i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1);     // beginning of current stack
        k2 = k1 + sectorCount + 1;      // beginning of next stack

        for(unsigned int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            // 2 triangles per sector excluding the first and last stacks

            // k1 => k2 => k1+1
            if(i != 0) {
                sphere->indices.push_back(k1);
                sphere->indices.push_back(k2);
                sphere->indices.push_back(k1 + 1);
            }

            // k1+1 => k2 => k2+1
            if(i != (stackCount-1)) {
                sphere->indices.push_back(k1 + 1);
                sphere->indices.push_back(k2);
                sphere->indices.push_back(k2 + 1);
            }
        }
    }

    // Generate tangents and bitangents
    computeTangentBasis(sphere);

    return sphere;
}

Mesh* MeshGen::createPlane(unsigned int resolutionX, unsigned int resolutionY, float width, float height) {
    Mesh* plane = new Mesh();

    // Clear any existing data
    plane->vertices.clear();
    plane->normals.clear();
    plane->uvs.clear();
    plane->indices.clear();

    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;

    // Generate a grid of vertices (resolution + 1 in each dimension to create all quads)
    for (unsigned int z = 0; z <= resolutionY; ++z) {
        for (unsigned int x = 0; x <= resolutionX; ++x) {
            // Calculate position for this vertex
            float posX = -halfWidth + ((float)x / resolutionX) * width;
            float posZ = -halfHeight + ((float)z / resolutionY) * height;

            // Add vertex
            plane->vertices.push_back(glm::vec3(posX, 0.0f, posZ));

            // Add normal
            plane->normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));

            // Add UV (normalized coordinates)
            plane->uvs.push_back(glm::vec2((float)x / resolutionX, (float)z / resolutionY));
        }
    }

    // Generate indices for the quads (made up of shared vertices)
    for (unsigned int z = 0; z < resolutionY; ++z) {
        for (unsigned int x = 0; x < resolutionX; ++x) {
            // Calculate indices for the corners of this quad
            unsigned int topLeft = z * (resolutionX + 1) + x;
            unsigned int topRight = topLeft + 1;
            unsigned int bottomLeft = (z + 1) * (resolutionX + 1) + x;
            unsigned int bottomRight = bottomLeft + 1;

            // First triangle (top-left, bottom-right, top-right)
            plane->indices.push_back(topLeft);
            plane->indices.push_back(bottomRight);
            plane->indices.push_back(topRight);

            // Second triangle (top-left, bottom-left, bottom-right)
            plane->indices.push_back(topLeft);
            plane->indices.push_back(bottomLeft);
            plane->indices.push_back(bottomRight);
        }
    }

    // Generate tangents and bitangents for texturing
    computeTangentBasis(plane);

    return plane;
}

// Helper function to compute tangent basis for normal mapping
void MeshGen::computeTangentBasis(Mesh* mesh) {
    mesh->tangents.resize(mesh->vertices.size(), glm::vec4(0.0f));

    std::vector<glm::vec3> accumulatedTangents(mesh->vertices.size(), glm::vec3(0.0f));
    std::vector<glm::vec3> accumulatedBitangents(mesh->vertices.size(), glm::vec3(0.0f));

    for (size_t i = 0; i + 2 < mesh->indices.size(); i += 3) {
        unsigned int idx1 = mesh->indices[i];
        unsigned int idx2 = mesh->indices[i + 1];
        unsigned int idx3 = mesh->indices[i + 2];

        const glm::vec3& v0 = mesh->vertices[idx1];
        const glm::vec3& v1 = mesh->vertices[idx2];
        const glm::vec3& v2 = mesh->vertices[idx3];

        const glm::vec2& uv0 = mesh->uvs[idx1];
        const glm::vec2& uv1 = mesh->uvs[idx2];
        const glm::vec2& uv2 = mesh->uvs[idx3];

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec2 deltaUV1 = uv1 - uv0;
        glm::vec2 deltaUV2 = uv2 - uv0;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        glm::vec3 tangent = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
        glm::vec3 bitangent = f * (-deltaUV2.x * edge1 + deltaUV1.x * edge2);

        accumulatedTangents[idx1] += tangent;
        accumulatedTangents[idx2] += tangent;
        accumulatedTangents[idx3] += tangent;

        accumulatedBitangents[idx1] += bitangent;
        accumulatedBitangents[idx2] += bitangent;
        accumulatedBitangents[idx3] += bitangent;
    }

    // Compute orthonormal basis and handedness
    for (size_t i = 0; i < mesh->vertices.size(); ++i) {
        const glm::vec3& n = mesh->normals[i];
        glm::vec3 t = accumulatedTangents[i];
        glm::vec3 b = accumulatedBitangents[i];

        // Gram-Schmidt orthogonalization
        t = glm::normalize(t - n * glm::dot(n, t));

        // Calculate handedness
        float handedness = (glm::dot(glm::cross(n, t), b) < 0.0f) ? -1.0f : 1.0f;

        mesh->tangents[i] = glm::vec4(t, handedness);
    }
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
