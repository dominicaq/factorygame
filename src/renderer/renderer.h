#ifndef RENDERER_H
#define RENDERER_H

#include "shader.h"
#include "../transform.h"
#include "../resources/mesh.h"

#include <glad/glad.h>

#include <memory>
#include <vector>

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
    void initMeshBuffers(Mesh* mesh);
    void deleteMeshBuffer(const Mesh* mesh);

    /*
    * G-Buffer management for deferred rendering
    */
    void resizeGBuffer(int width, int height);
    void geometryPass(const std::vector<Mesh*>& meshes,
        const std::vector<Transform>& transforms,
        const glm::mat4& view,
        const glm::mat4& projection);

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
    * Render Quad
    */
    unsigned int m_quadVAO;

    /*
    * G-buffer resources
    */
    unsigned int m_gBuffer;
    unsigned int m_gPosition;
    unsigned int m_gNormal;
    unsigned int m_gAlbedo;
    unsigned int m_rboDepth;

    Shader m_gBufferShader;

    /*
    * Mesh buffer storage
    */
    struct MeshData {
        unsigned int VAO, VBO, EBO;
        GLsizei indexCount;
        GLsizei vertexCount;
    };

    std::vector<MeshData> m_meshData;
};

#endif // RENDERER_H
