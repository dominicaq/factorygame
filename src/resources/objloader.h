#pragma once

#include "../components/mesh.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

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
inline void generateUVs(Mesh* mesh);
inline void computePackedNormalTangents(Mesh* mesh);

/*
*  Main OBJ Loading
*/
static Mesh* loadOBJ(const std::string& fileContent) {
    if (fileContent.empty()) {
        std::cerr << "[Error] OBJLoader::loadOBJ: File content is empty\n";
        return nullptr;
    }

    Mesh* mesh = new Mesh();
    OBJParseContext ctx(*mesh);

    std::istringstream fileStream(fileContent);
    std::string line;
    while (std::getline(fileStream, line)) {
        parseOBJLine(line, ctx);
    }

    // Check if we have any vertices
    if (mesh->vertices.empty()) {
        std::cerr << "[Error] OBJLoader::loadOBJ: No vertices found in OBJ file\n";
        delete mesh;
        return nullptr;
    }

    // Generate temporary normal and tangent vectors
    std::vector<glm::vec3> tempNormals;
    std::vector<glm::vec4> tempTangents;

    // Generate UVs if they were not present in the file or are empty
    if (mesh->uvs.empty() || (mesh->uvs.size() > 0 && mesh->uvs[0] == glm::vec2(0.0f))) {
        generateUVs(mesh);
    }

    computePackedNormalTangents(mesh);
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
    auto it = ctx.vertexMap.find(vi);
    if (it != ctx.vertexMap.end()) {
        return it->second;
    }

    // Add new vertex
    ctx.mesh.vertices.push_back(ctx.temp_vertices[vi.vertex_idx]);

    // If UVs are present, add them, otherwise add placeholder
    if (vi.uv_idx >= 0 && vi.uv_idx < static_cast<int>(ctx.temp_uvs.size())) {
        ctx.mesh.uvs.push_back(ctx.temp_uvs[vi.uv_idx]);
    } else {
        ctx.mesh.uvs.push_back(glm::vec2(0.0f)); // UVs will be generated later
    }

    unsigned int new_idx = static_cast<unsigned int>(ctx.mesh.vertices.size() - 1);
    ctx.vertexMap[vi] = new_idx;

    return new_idx;
}

static void parseOBJLine(const std::string& line, OBJParseContext& ctx) {
    // Skip empty lines and comments
    if (line.empty() || line[0] == '#') {
        return;
    }

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

            // Validate vertex index
            if (vertex_idx == 0 || vertex_idx > ctx.temp_vertices.size()) {
                std::cerr << "[Warning] Invalid vertex index: " << vertex_idx << std::endl;
                continue;
            }

            // Adjust indices to be zero-based
            VertexIdx vi = {
                vertex_idx - 1,
                uv_idx > 0 ? uv_idx - 1 : -1,
                normal_idx > 0 ? normal_idx - 1 : -1
            };

            unsigned int idx = processVertex(vi, ctx);
            face_indices.push_back(idx);
        }

        // Triangulate face (fan triangulation)
        if (face_indices.size() >= 3) {
            for (size_t i = 1; i + 1 < face_indices.size(); i++) {
                ctx.mesh.indices.push_back(face_indices[0]);
                ctx.mesh.indices.push_back(face_indices[i]);
                ctx.mesh.indices.push_back(face_indices[i + 1]);
            }
        }
    }
}

/*
*  Mesh data generators
*/
inline void generateUVs(Mesh* mesh) {
    mesh->uvs.clear();
    mesh->uvs.resize(mesh->vertices.size());

    // Find bounding box for better UV mapping
    glm::vec3 minBounds = mesh->vertices[0];
    glm::vec3 maxBounds = mesh->vertices[0];

    for (const auto& vertex : mesh->vertices) {
        minBounds = glm::min(minBounds, vertex);
        maxBounds = glm::max(maxBounds, vertex);
    }

    glm::vec3 size = maxBounds - minBounds;
    float maxSize = std::max({size.x, size.y, size.z});

    for (size_t i = 0; i < mesh->vertices.size(); ++i) {
        // Planar projection onto XY plane with proper scaling
        float u = (mesh->vertices[i].x - minBounds.x) / maxSize;
        float v = (mesh->vertices[i].y - minBounds.y) / maxSize;
        mesh->uvs[i] = glm::vec2(u, v);
    }
}

inline void packTBNframe(Mesh* mesh, const std::vector<glm::vec3>& normals,
             const std::vector<glm::vec4>& tangents) {
    size_t count = std::min(normals.size(), tangents.size());
    mesh->packedNormalTangents.reserve(mesh->packedNormalTangents.size() + count);

    constexpr int storageSize = 2; // sizeof(int16_t)
    constexpr float bias = 1.0f / ((1 << (storageSize * 8 - 1)) - 1);

    for (size_t i = 0; i < count; ++i) {
        const glm::vec3& n = normals[i];
        const glm::vec4& t = tangents[i];
        glm::vec3 tangent(t.x, t.y, t.z);

        // Compute cross product: c = n × t (CRITICAL: this is the correct order)
        glm::vec3 c = glm::cross(n, tangent);

        // Construct matrix in COLUMN-MAJOR order: [tangent, c, normal]
        // This matches the JavaScript toMat3 function exactly
        glm::mat3 tbn(
            tangent.x, tangent.y, tangent.z,  // First column: tangent
            c.x,       c.y,       c.z,        // Second column: cross product
            n.x,       n.y,       n.z         // Third column: normal
        );

        // Extract quaternion and normalize
        glm::quat q = glm::normalize(glm::quat_cast(tbn));

        // Ensure positive quaternion (w >= 0)
        if (q.w < 0.0f) {
            q = -q;
        }

        // Apply bias to prevent w from being zero
        if (q.w < bias) {
            q.w = bias;
            float factor = std::sqrt(1.0f - bias * bias);
            q.x *= factor;
            q.y *= factor;
            q.z *= factor;
        }

        // Compute bitangent based on handedness for reflection check
        glm::vec3 b;
        if (t.w > 0.0f) {
            b = glm::cross(tangent, n);
        } else {
            b = glm::cross(n, tangent);
        }

        // Reflection check: if (n × t) · b < 0, negate quaternion
        glm::vec3 cc = glm::cross(tangent, n);
        if (glm::dot(cc, b) < 0.0f) {
            q = -q;
        }

        // Store packed quaternion
        mesh->packedNormalTangents.push_back(glm::vec4(q.x, q.y, q.z, q.w));
    }
}

void computePackedNormalTangents(Mesh* mesh) {
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

    packTBNframe(mesh, normals, tangents);
}

} // namespace ObjLoader
