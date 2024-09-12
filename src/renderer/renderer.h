#pragma once

#include <unordered_map>
#include <glad/glad.h>
#include "shader.h"
#include "../resources/mesh.h"
#include <memory>
#include <map>

class Renderer {
public:
    Renderer(unsigned int width, unsigned int height);
    ~Renderer();

    void draw(const Mesh* mesh, const Shader& shader);
    void initMeshBuffers(const Mesh* mesh);
    void deleteMeshBuffer(const Mesh* mesh);

    /*
    * GBuffer passes
    */
    void geometryPass(const Shader& geometryShader);

private:
    void initOpenGLState();
    void initGBuffer(unsigned int width, unsigned int height);

    // G-buffer
    unsigned int m_gBuffer;
    unsigned int m_gPosition, m_gNormal, m_gAlbedo;
    unsigned int m_rboDepth;

    // MeshData structure
    struct MeshData {
        unsigned int VAO, VBO, EBO;
    };

    // Store mesh buffers
    std::map<const Mesh*, MeshData> m_meshBuffers;

    // Mesh generation helpers
    void generateNormals(const Mesh* mesh, std::vector<glm::vec3>& normals, const std::vector<unsigned int>& indices);
    void generateUVs(const Mesh* mesh, std::vector<glm::vec2>& uvs);
};
