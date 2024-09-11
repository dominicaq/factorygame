#pragma once

#include <unordered_map>
#include <glad/glad.h>

#include "shader.h"
#include "../resources/mesh.h"

#include <memory>

class Renderer {
public:
    Renderer(unsigned int width, unsigned int height);
    ~Renderer();

    // Mesh bufers
    void initMeshBuffers(const Mesh* mesh);
    void deleteMeshBuffer(const Mesh* mesh);
    void draw(const Mesh* mesh, const Shader& shader);

private:
    struct MeshData {
        GLuint VAO;
        GLuint VBO;
        GLuint EBO;
    };

    // Generate missing mesh data
    void generateNormals(const Mesh* mesh, std::vector<glm::vec3>& normals, const std::vector<unsigned int>& indices);
    void generateUVs(const Mesh* mesh, std::vector<glm::vec2>& uvs);

    std::unordered_map<const Mesh*, MeshData> m_meshBuffers;
    void initOpenGLState();
};
