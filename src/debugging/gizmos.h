#ifndef GIZMOS_H
#define GIZMOS_H

#include "../components/mesh.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <vector>

namespace Gizmos {

// Helper function to pack normal and tangent into a quaternion
static glm::quat packNormalTangent(const glm::vec3& normal, const glm::vec3& tangent) {
    // Normalize inputs
    glm::vec3 n = glm::normalize(normal);
    glm::vec3 t = glm::normalize(tangent);

    // Compute bitangent
    glm::vec3 b = glm::cross(n, t);

    // Create orthonormal basis matrix
    glm::mat3 tbn(t, b, n);

    // Convert to quaternion
    return glm::quat_cast(tbn);
}

static Mesh* createCube() {
    Mesh* mesh = new Mesh();

    // Set cube vertices (8 vertices)
    // Bottom face vertices
    mesh->vertices.push_back(glm::vec3(-0.5f, -0.5f, -0.5f));  // 0: bottom-left-back
    mesh->vertices.push_back(glm::vec3(0.5f, -0.5f, -0.5f));   // 1: bottom-right-back
    mesh->vertices.push_back(glm::vec3(0.5f, -0.5f, 0.5f));    // 2: bottom-right-front
    mesh->vertices.push_back(glm::vec3(-0.5f, -0.5f, 0.5f));   // 3: bottom-left-front

    // Top face vertices
    mesh->vertices.push_back(glm::vec3(-0.5f, 0.5f, -0.5f));   // 4: top-left-back
    mesh->vertices.push_back(glm::vec3(0.5f, 0.5f, -0.5f));    // 5: top-right-back
    mesh->vertices.push_back(glm::vec3(0.5f, 0.5f, 0.5f));     // 6: top-right-front
    mesh->vertices.push_back(glm::vec3(-0.5f, 0.5f, 0.5f));    // 7: top-left-front

    // Set indices for wireframe lines (12 edges)
    // Bottom face edges
    mesh->indices.push_back(0); mesh->indices.push_back(1);
    mesh->indices.push_back(1); mesh->indices.push_back(2);
    mesh->indices.push_back(2); mesh->indices.push_back(3);
    mesh->indices.push_back(3); mesh->indices.push_back(0);

    // Top face edges
    mesh->indices.push_back(4); mesh->indices.push_back(5);
    mesh->indices.push_back(5); mesh->indices.push_back(6);
    mesh->indices.push_back(6); mesh->indices.push_back(7);
    mesh->indices.push_back(7); mesh->indices.push_back(4);

    // Vertical edges connecting top and bottom faces
    mesh->indices.push_back(0); mesh->indices.push_back(4);
    mesh->indices.push_back(1); mesh->indices.push_back(5);
    mesh->indices.push_back(2); mesh->indices.push_back(6);
    mesh->indices.push_back(3); mesh->indices.push_back(7);

    // Pack normals and tangents into quaternions
    std::vector<glm::vec3> normals = {
        glm::vec3(-1.0f, -1.0f, -1.0f),
        glm::vec3(1.0f, -1.0f, -1.0f),
        glm::vec3(1.0f, -1.0f, 1.0f),
        glm::vec3(-1.0f, -1.0f, 1.0f),
        glm::vec3(-1.0f, 1.0f, -1.0f),
        glm::vec3(1.0f, 1.0f, -1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(-1.0f, 1.0f, 1.0f)
    };

    for (size_t i = 0; i < normals.size(); i++) {
        glm::vec3 tangent(1.0f, 0.0f, 0.0f);
        glm::quat packed = packNormalTangent(normals[i], tangent);
        mesh->packedTNBFrame.push_back(glm::vec4(packed.x, packed.y, packed.z, packed.w));
    }

    // Set UVs
    mesh->uvs.push_back(glm::vec2(0.0f, 0.0f));
    mesh->uvs.push_back(glm::vec2(1.0f, 0.0f));
    mesh->uvs.push_back(glm::vec2(1.0f, 1.0f));
    mesh->uvs.push_back(glm::vec2(0.0f, 1.0f));
    mesh->uvs.push_back(glm::vec2(0.0f, 0.0f));
    mesh->uvs.push_back(glm::vec2(1.0f, 0.0f));
    mesh->uvs.push_back(glm::vec2(1.0f, 1.0f));
    mesh->uvs.push_back(glm::vec2(0.0f, 1.0f));

    return mesh;
}

/*
* Creates a square gizmo on the XY plane, centered at the origin
* with a side length of 1.0
*/
static Mesh* createSquare() {
    Mesh* mesh = new Mesh();

    // Set vertices (square on XY plane)
    mesh->vertices.push_back(glm::vec3(-0.5f, -0.5f, 0.0f));  // Bottom left
    mesh->vertices.push_back(glm::vec3(0.5f, -0.5f, 0.0f));   // Bottom right
    mesh->vertices.push_back(glm::vec3(0.5f, 0.5f, 0.0f));    // Top right
    mesh->vertices.push_back(glm::vec3(-0.5f, 0.5f, 0.0f));   // Top left

    // Set indices for two triangles
    mesh->indices.push_back(0);
    mesh->indices.push_back(1);
    mesh->indices.push_back(2);

    mesh->indices.push_back(0);
    mesh->indices.push_back(2);
    mesh->indices.push_back(3);

    // Pack normals and tangents into quaternions (all pointing in +Z direction)
    glm::vec3 normal(0.0f, 0.0f, 1.0f);
    glm::vec3 tangent(1.0f, 0.0f, 0.0f);
    glm::quat packed = packNormalTangent(normal, tangent);
    glm::vec4 packedVec4(packed.x, packed.y, packed.z, packed.w);

    for (size_t i = 0; i < mesh->vertices.size(); i++) {
        mesh->packedTNBFrame.push_back(packedVec4);
    }

    // Set UVs
    mesh->uvs.push_back(glm::vec2(0.0f, 0.0f));  // Bottom left
    mesh->uvs.push_back(glm::vec2(1.0f, 0.0f));  // Bottom right
    mesh->uvs.push_back(glm::vec2(1.0f, 1.0f));  // Top right
    mesh->uvs.push_back(glm::vec2(0.0f, 1.0f));  // Top left

    return mesh;
}

/*
* Creates a plane gizmo on the XZ plane, centered at the origin
* with dimensions of 1.0 x 1.0
*/
static Mesh* createPlane(float width = 1.0f, float length = 1.0f, int widthSegments = 1, int lengthSegments = 1) {
    Mesh* mesh = new Mesh();

    float halfWidth = width / 2.0f;
    float halfLength = length / 2.0f;

    // Create vertices grid
    for (int z = 0; z <= lengthSegments; z++) {
        for (int x = 0; x <= widthSegments; x++) {
            float xPos = (-halfWidth) + (float)x / (float)widthSegments * width;
            float zPos = (-halfLength) + (float)z / (float)lengthSegments * length;

            // Position
            mesh->vertices.push_back(glm::vec3(xPos, 0.0f, zPos));

            // UV
            mesh->uvs.push_back(glm::vec2(
                (float)x / (float)widthSegments,
                (float)z / (float)lengthSegments
            ));
        }
    }

    // Create indices for triangles
    for (int z = 0; z < lengthSegments; z++) {
        for (int x = 0; x < widthSegments; x++) {
            int topLeft = z * (widthSegments + 1) + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * (widthSegments + 1) + x;
            int bottomRight = bottomLeft + 1;

            // First triangle
            mesh->indices.push_back(topLeft);
            mesh->indices.push_back(bottomLeft);
            mesh->indices.push_back(topRight);

            // Second triangle
            mesh->indices.push_back(topRight);
            mesh->indices.push_back(bottomLeft);
            mesh->indices.push_back(bottomRight);
        }
    }

    // Pack normals and tangents into quaternions (normal pointing up, tangent along X)
    glm::vec3 normal(0.0f, 1.0f, 0.0f);
    glm::vec3 tangent(1.0f, 0.0f, 0.0f);
    glm::quat packed = packNormalTangent(normal, tangent);
    glm::vec4 packedVec4(packed.x, packed.y, packed.z, packed.w);

    for (size_t i = 0; i < mesh->vertices.size(); i++) {
        mesh->packedTNBFrame.push_back(packedVec4);
    }

    return mesh;
}

/*
* Creates a grid gizmo on the XZ plane, centered at the origin
*/
static Mesh* createGrid(float size = 10.0f, int divisions = 10) {
    Mesh* mesh = new Mesh();

    float halfSize = size / 2.0f;
    float step = size / (float)divisions;

    int lineCount = divisions + 1;

    // Create X lines
    for (int i = 0; i < lineCount; i++) {
        float pos = -halfSize + i * step;

        // Line along X axis
        mesh->vertices.push_back(glm::vec3(-halfSize, 0.0f, pos));
        mesh->vertices.push_back(glm::vec3(halfSize, 0.0f, pos));

        // Line along Z axis
        mesh->vertices.push_back(glm::vec3(pos, 0.0f, -halfSize));
        mesh->vertices.push_back(glm::vec3(pos, 0.0f, halfSize));
    }

    // Create indices (for lines)
    for (unsigned int i = 0; i < mesh->vertices.size(); i++) {
        mesh->indices.push_back(i);
    }

    // Pack normals and tangents into quaternions
    glm::vec3 normal(0.0f, 1.0f, 0.0f);
    glm::vec3 tangent(1.0f, 0.0f, 0.0f);
    glm::quat packed = packNormalTangent(normal, tangent);
    glm::vec4 packedVec4(packed.x, packed.y, packed.z, packed.w);

    for (size_t i = 0; i < mesh->vertices.size(); i++) {
        mesh->packedTNBFrame.push_back(packedVec4);
    }

    // Add UVs (not really needed for wireframes, but might be expected)
    for (size_t i = 0; i < mesh->vertices.size(); i++) {
        mesh->uvs.push_back(glm::vec2(0.0f, 0.0f));
    }

    return mesh;
}

/*
* Creates an axis gizmo showing X, Y, Z axes
*/
static Mesh* createAxis(float size = 1.0f) {
    Mesh* mesh = new Mesh();

    // Create vertices for axis lines
    // X axis (red)
    mesh->vertices.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
    mesh->vertices.push_back(glm::vec3(size, 0.0f, 0.0f));

    // Y axis (green)
    mesh->vertices.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
    mesh->vertices.push_back(glm::vec3(0.0f, size, 0.0f));

    // Z axis (blue)
    mesh->vertices.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
    mesh->vertices.push_back(glm::vec3(0.0f, 0.0f, size));

    // Set indices for lines
    for (unsigned int i = 0; i < mesh->vertices.size(); i++) {
        mesh->indices.push_back(i);
    }

    // Pack normals and tangents into quaternions
    // X axis vertices
    glm::quat xAxisPacked = packNormalTangent(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    mesh->packedTNBFrame.push_back(glm::vec4(xAxisPacked.x, xAxisPacked.y, xAxisPacked.z, xAxisPacked.w));
    mesh->packedTNBFrame.push_back(glm::vec4(xAxisPacked.x, xAxisPacked.y, xAxisPacked.z, xAxisPacked.w));

    // Y axis vertices
    glm::quat yAxisPacked = packNormalTangent(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    mesh->packedTNBFrame.push_back(glm::vec4(yAxisPacked.x, yAxisPacked.y, yAxisPacked.z, yAxisPacked.w));
    mesh->packedTNBFrame.push_back(glm::vec4(yAxisPacked.x, yAxisPacked.y, yAxisPacked.z, yAxisPacked.w));

    // Z axis vertices
    glm::quat zAxisPacked = packNormalTangent(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    mesh->packedTNBFrame.push_back(glm::vec4(zAxisPacked.x, zAxisPacked.y, zAxisPacked.z, zAxisPacked.w));
    mesh->packedTNBFrame.push_back(glm::vec4(zAxisPacked.x, zAxisPacked.y, zAxisPacked.z, zAxisPacked.w));

    // Add UVs (not really needed for lines)
    for (size_t i = 0; i < mesh->vertices.size(); i++) {
        mesh->uvs.push_back(glm::vec2(0.0f, 0.0f));
    }

    return mesh;
}

} // namespace Gizmos

#endif // GIZMOS_H
