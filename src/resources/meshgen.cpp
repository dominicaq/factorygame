#include "meshgen.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

Mesh* MeshGen::createCube() {
    Mesh* cubeMesh = new Mesh();

    // Positions for the cube (properly organized by face)
    cubeMesh->vertices = {
        // Front face
        { -0.5f, -0.5f,  0.5f }, {  0.5f, -0.5f,  0.5f }, {  0.5f,  0.5f,  0.5f }, { -0.5f,  0.5f,  0.5f },
        // Back face
        {  0.5f, -0.5f, -0.5f }, { -0.5f, -0.5f, -0.5f }, { -0.5f,  0.5f, -0.5f }, {  0.5f,  0.5f, -0.5f },
        // Left face
        { -0.5f, -0.5f, -0.5f }, { -0.5f, -0.5f,  0.5f }, { -0.5f,  0.5f,  0.5f }, { -0.5f,  0.5f, -0.5f },
        // Right face
        {  0.5f, -0.5f,  0.5f }, {  0.5f, -0.5f, -0.5f }, {  0.5f,  0.5f, -0.5f }, {  0.5f,  0.5f,  0.5f },
        // Bottom face
        { -0.5f, -0.5f, -0.5f }, {  0.5f, -0.5f, -0.5f }, {  0.5f, -0.5f,  0.5f }, { -0.5f, -0.5f,  0.5f },
        // Top face
        { -0.5f,  0.5f,  0.5f }, {  0.5f,  0.5f,  0.5f }, {  0.5f,  0.5f, -0.5f }, { -0.5f,  0.5f, -0.5f }
    };

    // UVs for the cube
    cubeMesh->uvs = {
        // Front face
        { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f },
        // Back face
        { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f },
        // Left face
        { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f },
        // Right face
        { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f },
        // Bottom face
        { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f },
        // Top face
        { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f }
    };

    // Indices for the cube (two triangles per face)
    cubeMesh->indices = {
        // Front face
        0, 1, 2,   2, 3, 0,
        // Back face
        4, 5, 6,   6, 7, 4,
        // Left face
        8, 9, 10,  10, 11, 8,
        // Right face
        12, 13, 14, 14, 15, 12,
        // Bottom face
        16, 17, 18, 18, 19, 16,
        // Top face
        20, 21, 22, 22, 23, 20
    };

    // Generate packed normal-tangents
    computePackedNormalTangents(cubeMesh);

    return cubeMesh;
}

Mesh* MeshGen::createQuad(float scale) {
    Mesh* quadMesh = new Mesh();

    // Positions for the quad (a flat square in the XY plane, scaled)
    quadMesh->vertices = {
        { -0.5f * scale, -0.5f * scale, 0.0f }, // Bottom-left
        {  0.5f * scale, -0.5f * scale, 0.0f }, // Bottom-right
        {  0.5f * scale,  0.5f * scale, 0.0f }, // Top-right
        { -0.5f * scale,  0.5f * scale, 0.0f }  // Top-left
    };

    // UVs for the quad
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

    // Generate packed normal-tangents
    computePackedNormalTangents(quadMesh);

    return quadMesh;
}

Mesh* MeshGen::createSphere(unsigned int sectorCount, unsigned int stackCount) {
    Mesh* sphere = new Mesh();

    const float radius = 1.0f;
    const float PI = 3.14159265358979323846f;

    // Clear any existing data
    sphere->vertices.clear();
    sphere->uvs.clear();
    sphere->indices.clear();

    float x, y, z, xy;                  // vertex position
    float s, t;                         // vertex texCoord

    float sectorStep = 2 * PI / sectorCount;
    float stackStep = PI / stackCount;
    float sectorAngle, stackAngle;

    // Compute all vertices first
    for(unsigned int i = 0; i <= stackCount; ++i) {
        stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);               // r * cos(u)
        z = radius * sinf(stackAngle);                // r * sin(u)

        // Add (sectorCount+1) vertices per stack
        for(unsigned int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep;             // starting from 0 to 2pi

            // vertex position
            x = xy * cosf(sectorAngle);               // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);               // r * cos(u) * sin(v)
            sphere->vertices.push_back(glm::vec3(x, y, z));

            // vertex tex coord between [0, 1]
            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            sphere->uvs.push_back(glm::vec2(s, t));
        }
    }

    // Generate indices
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

    // Generate packed normal-tangents
    computePackedNormalTangents(sphere);

    return sphere;
}

Mesh* MeshGen::createPlane(unsigned int resolutionX, unsigned int resolutionY, float width, float height) {
    Mesh* plane = new Mesh();

    // Clear any existing data
    plane->vertices.clear();
    plane->uvs.clear();
    plane->indices.clear();

    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;

    // Generate a grid of vertices
    for (unsigned int z = 0; z <= resolutionY; ++z) {
        for (unsigned int x = 0; x <= resolutionX; ++x) {
            // Calculate position for this vertex
            float posX = -halfWidth + ((float)x / resolutionX) * width;
            float posZ = -halfHeight + ((float)z / resolutionY) * height;

            // Add vertex
            plane->vertices.push_back(glm::vec3(posX, 0.0f, posZ));

            // Add UV (normalized coordinates)
            plane->uvs.push_back(glm::vec2((float)x / resolutionX, (float)z / resolutionY));
        }
    }

    // Generate indices for the quads
    for (unsigned int z = 0; z < resolutionY; ++z) {
        for (unsigned int x = 0; x < resolutionX; ++x) {
            // Calculate indices for the corners of this quad
            uint32_t topLeft = z * (resolutionX + 1) + x;
            uint32_t topRight = topLeft + 1;
            uint32_t bottomLeft = (z + 1) * (resolutionX + 1) + x;
            uint32_t bottomRight = bottomLeft + 1;

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

    // Generate packed normal-tangents
    computePackedNormalTangents(plane);

    return plane;
}

// Helper function to compute packed normal-tangent quaternions
void MeshGen::computePackedNormalTangents(Mesh* mesh) {
    size_t vertexCount = mesh->vertices.size();

    // Temporary storage for normals and tangents
    std::vector<glm::vec3> normals(vertexCount, glm::vec3(0.0f));
    std::vector<glm::vec4> tangents(vertexCount, glm::vec4(0.0f));

    std::vector<glm::vec3> accumulatedTangents(vertexCount, glm::vec3(0.0f));
    std::vector<glm::vec3> accumulatedBitangents(vertexCount, glm::vec3(0.0f));

    // Calculate face normals and accumulate them
    for (size_t i = 0; i < mesh->indices.size(); i += 3) {
        if (i + 2 >= mesh->indices.size()) break;

        uint32_t idx0 = mesh->indices[i];
        uint32_t idx1 = mesh->indices[i + 1];
        uint32_t idx2 = mesh->indices[i + 2];

        if (idx0 >= vertexCount || idx1 >= vertexCount || idx2 >= vertexCount)
            continue;

        const glm::vec3& v0 = mesh->vertices[idx0];
        const glm::vec3& v1 = mesh->vertices[idx1];
        const glm::vec3& v2 = mesh->vertices[idx2];

        const glm::vec2& uv0 = mesh->uvs[idx0];
        const glm::vec2& uv1 = mesh->uvs[idx1];
        const glm::vec2& uv2 = mesh->uvs[idx2];

        // Calculate face normal
        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 faceNormal = glm::cross(edge1, edge2);

        float length = glm::length(faceNormal);
        if (length > 1e-6f) {
            faceNormal = faceNormal / length;
            normals[idx0] += faceNormal;
            normals[idx1] += faceNormal;
            normals[idx2] += faceNormal;
        }

        // Calculate tangent and bitangent
        glm::vec2 deltaUV1 = uv1 - uv0;
        glm::vec2 deltaUV2 = uv2 - uv0;

        float det = deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x;
        if (std::abs(det) > 1e-6f) {
            float invDet = 1.0f / det;

            glm::vec3 tangent = (edge1 * deltaUV2.y - edge2 * deltaUV1.y) * invDet;
            glm::vec3 bitangent = (edge2 * deltaUV1.x - edge1 * deltaUV2.x) * invDet;

            accumulatedTangents[idx0] += tangent;
            accumulatedTangents[idx1] += tangent;
            accumulatedTangents[idx2] += tangent;

            accumulatedBitangents[idx0] += bitangent;
            accumulatedBitangents[idx1] += bitangent;
            accumulatedBitangents[idx2] += bitangent;
        }
    }

    // Normalize normals and compute orthogonalized tangents
    for (size_t i = 0; i < vertexCount; ++i) {
        // Normalize normal
        float normalLength = glm::length(normals[i]);
        if (normalLength > 1e-6f) {
            normals[i] = normals[i] / normalLength;
        } else {
            // Fallback normal
            normals[i] = glm::vec3(0.0f, 1.0f, 0.0f);
        }

        const glm::vec3& n = normals[i];
        glm::vec3 t = accumulatedTangents[i];
        glm::vec3 b = accumulatedBitangents[i];

        // Gram-Schmidt orthogonalization
        float tLength = glm::length(t);
        if (tLength > 1e-6f) {
            t = t / tLength;
            t = t - n * glm::dot(n, t);
            tLength = glm::length(t);
            if (tLength > 1e-6f) {
                t = t / tLength;
            } else {
                // Fallback tangent
                glm::vec3 c1 = glm::cross(n, glm::vec3(0.0f, 0.0f, 1.0f));
                glm::vec3 c2 = glm::cross(n, glm::vec3(0.0f, 1.0f, 0.0f));
                t = glm::length(c1) > glm::length(c2) ? glm::normalize(c1) : glm::normalize(c2);
            }
        } else {
            // Fallback tangent
            glm::vec3 c1 = glm::cross(n, glm::vec3(0.0f, 0.0f, 1.0f));
            glm::vec3 c2 = glm::cross(n, glm::vec3(0.0f, 1.0f, 0.0f));
            t = glm::length(c1) > glm::length(c2) ? glm::normalize(c1) : glm::normalize(c2);
        }

        // Calculate handedness using the original accumulated bitangent
        glm::vec3 computedBitangent = glm::cross(n, t);
        float w = (glm::dot(computedBitangent, b) < 0.0f) ? -1.0f : 1.0f;

        tangents[i] = glm::vec4(t, w);
    }

    // Pack normals and tangents into quaternions
    mesh->packedNormalTangents.clear();
    mesh->packedNormalTangents.reserve(vertexCount);

    for (size_t i = 0; i < vertexCount; ++i) {
        glm::vec3 normal = glm::normalize(normals[i]);
        glm::vec3 tangent = glm::normalize(glm::vec3(tangents[i]));
        float handedness = tangents[i].w;

        // Create orthonormal basis - bitangent should be computed as cross(normal, tangent)
        // This gives us the "default" bitangent direction
        glm::vec3 bitangent = glm::normalize(glm::cross(normal, tangent));

        // Create TBN matrix with proper column order
        // Each column represents a basis vector in world space
        glm::mat3 tbnMatrix;
        tbnMatrix[0] = tangent;   // First column: tangent
        tbnMatrix[1] = bitangent; // Second column: bitangent
        tbnMatrix[2] = normal;    // Third column: normal

        // Convert to quaternion
        glm::quat quat = glm::quat_cast(tbnMatrix);

        // Ensure quaternion is in positive hemisphere for consistent encoding
        if (quat.w < 0.0f) {
            quat = -quat;
        }

        // Encode handedness in the sign of the w component
        if (handedness < 0.0f) {
            quat.w = -std::abs(quat.w);  // Make w negative to encode negative handedness
        } else {
            quat.w = std::abs(quat.w);   // Ensure w is positive for positive handedness
        }

        // Store quaternion components
        mesh->packedNormalTangents.push_back(glm::vec4(quat.x, quat.y, quat.z, quat.w));
    }
}

Mesh* MeshGen::createCapsule(float radius, float height, int sectors, int stacks) {
    Mesh* capsule = new Mesh();

    // TODO: Implement capsule generation
    // For now, return an empty mesh

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
