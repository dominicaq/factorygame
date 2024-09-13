#ifndef RENDERER_H
#define RENDERER_H

#include <unordered_map>
#include <glad/glad.h>

#include "../transform.h"

#include "shader.h"
#include "../resources/mesh.h"
#include <memory>
#include <map>

/*
* The Renderer class is responsible for handling OpenGL rendering, including
* G-buffer management for deferred rendering and mesh rendering.
*/
class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();

    /*
    * Render mesh using its buffers
    */
    void draw(const Mesh* mesh);

    /*
    * Initialize and manage mesh buffers
    */
    void initMeshBuffers(const Mesh* mesh);
    void deleteMeshBuffer(const Mesh* mesh);

    /*
    * G-Buffer management for deferred rendering
    */
    void resizeGBuffer(int width, int height);
    void geometryPass(const Shader& shader, const std::vector<Mesh*>& meshes, const std::vector<Transform>& transforms);

    /*
    * Quad rendering (used for post-processing, G-buffer display, etc.)
    */
    void drawQuad();
    void initQuad();

    /*
    * Debugging: Display G-buffer textures (e.g., Position, Normal, Albedo)
    */
    void debugGBuffer(const Shader& debugShader, int debugMode);

private:
    void initOpenGLState();
    void initGBuffer(int width, int height);

    /*
    * G-buffer resources
    */
    unsigned int m_gBuffer;
    unsigned int m_gPosition, m_gNormal, m_gAlbedo;
    unsigned int m_rboDepth;

    /*
    * Screen-aligned quad for G-buffer rendering and post-processing
    */
    unsigned int m_quadVAO;

    /*
    * Mesh buffer storage (mapping each Mesh* to its respective MeshData)
    */
    struct MeshData {
        unsigned int VAO, VBO, EBO;
    };
    std::map<const Mesh*, MeshData> m_meshBuffers;

    /*
    * Mesh attribute generation helpers
    */
    void generateNormals(const Mesh* mesh, std::vector<glm::vec3>& normals, const std::vector<unsigned int>& indices);
    void generateUVs(const Mesh* mesh, std::vector<glm::vec2>& uvs);
};

#endif
