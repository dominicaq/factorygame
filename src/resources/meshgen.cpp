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
    // Resize tangents and bitangents arrays
    mesh->tangents.resize(mesh->vertices.size(), glm::vec3(0.0f));
    mesh->bitangents.resize(mesh->vertices.size(), glm::vec3(0.0f));

    // Compute tangents and bitangents for each triangle
    for (size_t i = 0; i < mesh->indices.size(); i += 3) {
        unsigned int idx1 = mesh->indices[i];
        unsigned int idx2 = mesh->indices[i+1];
        unsigned int idx3 = mesh->indices[i+2];

        // Get triangle vertices
        const glm::vec3& v1 = mesh->vertices[idx1];
        const glm::vec3& v2 = mesh->vertices[idx2];
        const glm::vec3& v3 = mesh->vertices[idx3];

        // Get triangle UVs
        const glm::vec2& uv1 = mesh->uvs[idx1];
        const glm::vec2& uv2 = mesh->uvs[idx2];
        const glm::vec2& uv3 = mesh->uvs[idx3];

        // Edges of the triangle
        glm::vec3 edge1 = v2 - v1;
        glm::vec3 edge2 = v3 - v1;

        // Differences in UV coordinates
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        glm::vec3 tangent;
        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent = glm::normalize(tangent);

        glm::vec3 bitangent;
        bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent = glm::normalize(bitangent);

        // Add to all three vertices of the triangle
        mesh->tangents[idx1] += tangent;
        mesh->tangents[idx2] += tangent;
        mesh->tangents[idx3] += tangent;

        mesh->bitangents[idx1] += bitangent;
        mesh->bitangents[idx2] += bitangent;
        mesh->bitangents[idx3] += bitangent;
    }

    // Normalize the accumulated tangents and bitangents
    for (size_t i = 0; i < mesh->vertices.size(); ++i) {
        mesh->tangents[i] = glm::normalize(mesh->tangents[i]);
        mesh->bitangents[i] = glm::normalize(mesh->bitangents[i]);
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
