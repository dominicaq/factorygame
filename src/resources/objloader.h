#ifndef OBJLOADER_H
#define OBJLOADER_H

#include "mesh.h"
#include <glm.hpp>

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <unordered_map>

namespace ObjLoader {

/*
*  Struct and Hashing
*/
struct VertexIdx {
    unsigned int vertex_idx;
    int uv_idx, normal_idx;

    bool operator==(const VertexIdx& other) const {
        return vertex_idx == other.vertex_idx &&
               uv_idx == other.uv_idx &&
               normal_idx == other.normal_idx;
    }
};

struct VertexIdxHash {
    size_t operator()(const VertexIdx& vi) const {
        return ((std::hash<unsigned int>()(vi.vertex_idx) ^
                (std::hash<int>()(vi.uv_idx) << 1)) >> 1) ^
               (std::hash<int>()(vi.normal_idx) << 1);
    }
};

/*
* Parsing context that holds temporary data and maps for OBJ parsing
*/
struct OBJParseContext {
    Mesh& mesh;
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;
    std::unordered_map<VertexIdx, unsigned int, VertexIdxHash> vertexMap;

    OBJParseContext(Mesh& m)
        : mesh(m) {}
};

/*
*  Forward Declarations
*/
static void parseVertexString(const std::string& vertex_string, unsigned int& vertex_idx, int& uv_idx, int& normal_idx);
static unsigned int processVertex(VertexIdx& vi, OBJParseContext& ctx);
static void parseOBJLine(const std::string& line, OBJParseContext& ctx);
inline void generateNormals(Mesh* mesh);
inline void generateUVs(Mesh* mesh);

/*
*  Main OBJ Loading
*/
static Mesh* loadOBJ(const std::string& fileContent) {
    if (fileContent.empty()) {
        std::cerr << "ERROR::OBJ_LOADER::File content is empty\n";
        return nullptr;
    }

    Mesh* mesh = new Mesh();
    OBJParseContext ctx(*mesh);

    std::istringstream fileStream(fileContent);
    std::string line;
    while (std::getline(fileStream, line)) {
        parseOBJLine(line, ctx);
    }

    // Generate UVs and normals if they were not present in the file
    if (mesh->uvs.empty() || mesh->uvs[0] == glm::vec2()) {
        generateUVs(mesh);
    }

    if (mesh->normals.empty() || mesh->normals[0] == glm::vec3()) {
        generateNormals(mesh);
    }

    return mesh;
}

/*
*  Parsing
*/
static void parseVertexString(const std::string& vertex_string, unsigned int& vertex_idx, int& uv_idx, int& normal_idx) {
    std::istringstream vertex_stream(vertex_string);

    vertex_stream >> vertex_idx;

    if (vertex_string.find('/') != std::string::npos) {
        char delimiter;
        vertex_stream >> delimiter;

        if (vertex_stream.peek() != '/') {
            vertex_stream >> uv_idx;
        }

        if (vertex_stream.peek() == '/') {
            vertex_stream >> delimiter;  // Move to normal if present
            vertex_stream >> normal_idx;
        }
    }
}

/*
* Function to handle a parsed vertex and add it to the mesh if it's new
*/
static unsigned int processVertex(VertexIdx& vi, OBJParseContext& ctx) {
    // Check if this vertex has already been processed
    if (ctx.vertexMap.find(vi) != ctx.vertexMap.end()) {
        return ctx.vertexMap[vi];
    }

    // Add new vertex
    ctx.mesh.vertices.push_back(ctx.temp_vertices[vi.vertex_idx]);

    // If UVs are present, reserve space but don't assign default values
    if (vi.uv_idx >= 0 && vi.uv_idx < static_cast<int>(ctx.temp_uvs.size())) {
        ctx.mesh.uvs.push_back(ctx.temp_uvs[vi.uv_idx]);
    } else {
        ctx.mesh.uvs.push_back(glm::vec2()); // UVs will be generated later
    }

    // If normals are present, reserve space but don't assign default values
    if (vi.normal_idx >= 0 && vi.normal_idx < static_cast<int>(ctx.temp_normals.size())) {
        ctx.mesh.normals.push_back(ctx.temp_normals[vi.normal_idx]);
    } else {
        ctx.mesh.normals.push_back(glm::vec3()); // Normals will be generated later
    }

    unsigned int new_idx = static_cast<unsigned int>(ctx.mesh.vertices.size() - 1);
    ctx.vertexMap[vi] = new_idx;

    return new_idx;
}

static void parseOBJLine(const std::string& line, OBJParseContext& ctx) {
    std::istringstream s(line);
    std::string token;
    s >> token;

    if (token == "v") {
        // Vertex position
        glm::vec3 vertex;
        s >> vertex.x >> vertex.y >> vertex.z;
        ctx.temp_vertices.push_back(vertex);
    } else if (token == "vt") {
        // Texture coordinate
        glm::vec2 uv;
        s >> uv.x >> uv.y;
        ctx.temp_uvs.push_back(uv);
    } else if (token == "vn") {
        // Vertex normal
        glm::vec3 normal;
        s >> normal.x >> normal.y >> normal.z;
        ctx.temp_normals.push_back(normal);
    } else if (token == "f") {
        // Face
        std::string vertex_string;
        std::vector<unsigned int> face_indices;

        while (s >> vertex_string) {
            unsigned int vertex_idx = 0;
            int uv_idx = -1;
            int normal_idx = -1;

            parseVertexString(vertex_string, vertex_idx, uv_idx, normal_idx);

            // Adjust indices to be zero-based
            VertexIdx vi = {
                vertex_idx - 1,
                uv_idx - 1,
                normal_idx - 1
            };

            unsigned int idx = processVertex(vi, ctx);
            face_indices.push_back(idx);
        }

        // Triangulate face
        for (size_t i = 1; i + 1 < face_indices.size(); i++) {
            ctx.mesh.indices.push_back(face_indices[0]);
            ctx.mesh.indices.push_back(face_indices[i]);
            ctx.mesh.indices.push_back(face_indices[i + 1]);
        }
    }
}

/*
*  Mesh data generators
*/
inline void generateNormals(Mesh* mesh) {
    mesh->normals.resize(mesh->vertices.size(), glm::vec3(0.0f));

    for (size_t i = 0; i < mesh->indices.size(); i += 3) {
        unsigned int idx0 = mesh->indices[i];
        unsigned int idx1 = mesh->indices[i + 1];
        unsigned int idx2 = mesh->indices[i + 2];

        glm::vec3& v0 = mesh->vertices[idx0];
        glm::vec3& v1 = mesh->vertices[idx1];
        glm::vec3& v2 = mesh->vertices[idx2];

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

        mesh->normals[idx0] += faceNormal;
        mesh->normals[idx1] += faceNormal;
        mesh->normals[idx2] += faceNormal;
    }

    // Normalize the normals
    for (size_t i = 0; i < mesh->normals.size(); ++i) {
        mesh->normals[i] = glm::normalize(mesh->normals[i]);
    }
}

inline void generateUVs(Mesh* mesh) {
    mesh->uvs.resize(mesh->vertices.size());

    for (size_t i = 0; i < mesh->vertices.size(); ++i) {
        float u = (mesh->vertices[i].x + 1.0f) * 0.5f;
        float v = (mesh->vertices[i].y + 1.0f) * 0.5f;
        mesh->uvs[i] = glm::vec2(u, v);
    }
}

} // namespace ObjLoader

#endif // OBJLOADER_H
