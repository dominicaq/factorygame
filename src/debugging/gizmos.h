#ifndef GIZMOS_H
#define GIZMOS_H

#include "../components/mesh.h"
#include <glm/glm.hpp>

#include <vector>

namespace Gizmos {

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

    // Set normals (pointing outward for each vertex)
    mesh->normals.push_back(glm::vec3(-1.0f, -1.0f, -1.0f));
    mesh->normals.push_back(glm::vec3(1.0f, -1.0f, -1.0f));
    mesh->normals.push_back(glm::vec3(1.0f, -1.0f, 1.0f));
    mesh->normals.push_back(glm::vec3(-1.0f, -1.0f, 1.0f));
    mesh->normals.push_back(glm::vec3(-1.0f, 1.0f, -1.0f));
    mesh->normals.push_back(glm::vec3(1.0f, 1.0f, -1.0f));
    mesh->normals.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
    mesh->normals.push_back(glm::vec3(-1.0f, 1.0f, 1.0f));

    // Set UVs
    mesh->uvs.push_back(glm::vec2(0.0f, 0.0f));
    mesh->uvs.push_back(glm::vec2(1.0f, 0.0f));
    mesh->uvs.push_back(glm::vec2(1.0f, 1.0f));
    mesh->uvs.push_back(glm::vec2(0.0f, 1.0f));
    mesh->uvs.push_back(glm::vec2(0.0f, 0.0f));
    mesh->uvs.push_back(glm::vec2(1.0f, 0.0f));
    mesh->uvs.push_back(glm::vec2(1.0f, 1.0f));
    mesh->uvs.push_back(glm::vec2(0.0f, 1.0f));

    // Generate tangents and bitangents
    for (size_t i = 0; i < mesh->vertices.size(); i++) {
        mesh->tangents.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
        mesh->bitangents.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
    }

    // Set wireframe flag to true
    mesh->wireframe = true;

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

    // Set normals (all pointing in +Z direction)
    for (size_t i = 0; i < mesh->vertices.size(); i++) {
        mesh->normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
    }

    // Set UVs
    mesh->uvs.push_back(glm::vec2(0.0f, 0.0f));  // Bottom left
    mesh->uvs.push_back(glm::vec2(1.0f, 0.0f));  // Bottom right
    mesh->uvs.push_back(glm::vec2(1.0f, 1.0f));  // Top right
    mesh->uvs.push_back(glm::vec2(0.0f, 1.0f));  // Top left

    // Generate tangents and bitangents for the mesh
    size_t numVertices = mesh->vertices.size();
    std::vector<glm::vec3> tangents(numVertices, glm::vec3(1.0f, 0.0f, 0.0f));
    std::vector<glm::vec3> bitangents(numVertices, glm::vec3(0.0f, 1.0f, 0.0f));

    mesh->tangents = tangents;
    mesh->bitangents = bitangents;

    // Set wireframe flag
    mesh->wireframe = true;

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

            // Normal (pointing up)
            mesh->normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));

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

    // Generate tangents and bitangents
    size_t numVertices = mesh->vertices.size();
    std::vector<glm::vec3> tangents(numVertices, glm::vec3(1.0f, 0.0f, 0.0f));
    std::vector<glm::vec3> bitangents(numVertices, glm::vec3(0.0f, 0.0f, 1.0f));

    mesh->tangents = tangents;
    mesh->bitangents = bitangents;

    // Set wireframe flag
    mesh->wireframe = true;

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

    // Set normals
    for (size_t i = 0; i < mesh->vertices.size(); i++) {
        mesh->normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
    }

    // Create indices (for lines)
    for (unsigned int i = 0; i < mesh->vertices.size(); i++) {
        mesh->indices.push_back(i);
    }

    // Add UVs (not really needed for wireframes, but our renderer might need them)
    for (size_t i = 0; i < mesh->vertices.size(); i++) {
        mesh->uvs.push_back(glm::vec2(0.0f, 0.0f));
    }

    // Generate tangents and bitangents
    size_t numVertices = mesh->vertices.size();
    std::vector<glm::vec3> tangents(numVertices, glm::vec3(1.0f, 0.0f, 0.0f));
    std::vector<glm::vec3> bitangents(numVertices, glm::vec3(0.0f, 0.0f, 1.0f));

    mesh->tangents = tangents;
    mesh->bitangents = bitangents;

    // Set wireframe flag
    mesh->wireframe = true;

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

    // Set normals (not really needed for lines, but our renderer expects them)
    mesh->normals.push_back(glm::vec3(1.0f, 0.0f, 0.0f));  // X axis
    mesh->normals.push_back(glm::vec3(1.0f, 0.0f, 0.0f));

    mesh->normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));  // Y axis
    mesh->normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));

    mesh->normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));  // Z axis
    mesh->normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));

    // Add UVs (not really needed for lines)
    for (size_t i = 0; i < mesh->vertices.size(); i++) {
        mesh->uvs.push_back(glm::vec2(0.0f, 0.0f));
    }

    // Generate tangents and bitangents
    size_t numVertices = mesh->vertices.size();
    std::vector<glm::vec3> tangents(numVertices, glm::vec3(1.0f, 0.0f, 0.0f));
    std::vector<glm::vec3> bitangents(numVertices, glm::vec3(0.0f, 1.0f, 0.0f));

    mesh->tangents = tangents;
    mesh->bitangents = bitangents;

    // Set wireframe flag
    mesh->wireframe = true;

    return mesh;
}

} // namespace Gizmos

#endif // GIZMOS_H
