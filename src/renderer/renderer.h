#ifndef RENDERER_H
#define RENDERER_H

#include "../engine.h"

#include "framebuffer.h"
#include "cube_map.h"

#include <glad/glad.h>
#include <memory>
#include <vector>

// Forward declarations to prevent circular dependency from engine.h
class Mesh;
class ECSWorld;
class Entity;
class Camera;

/*
 * The Renderer class is responsible for handling OpenGL rendering,
 * including G-buffer management for deferred rendering and mesh rendering.
 */
class Renderer {
public:
    Renderer(int width, int height, Camera* camera);
    ~Renderer();

    /*
    * Getters
    */
    std::pair<int, int> getScreenDimensions() const {
        return {m_width, m_height};
    }
    Framebuffer* getFramebuffer() const { return m_gBuffer.get(); }
    Camera* getCamera() const { return m_camera; }

    /*
     * Initialize and manage mesh buffers
     */
    void draw(const Mesh* mesh);
    void initMeshBuffers(Mesh* mesh, bool isStatic = true);
    void deleteMeshBuffer(const Mesh* mesh);

    /*
     * Quad rendering (used for post-processing, G-buffer display, etc.)
     */
    void drawScreenQuad();

    /*
     * Recreate G-buffer with new dimensions
     */
    void resizeGBuffer(int width, int height);

    /*
     * Debugging: Display G-buffer textures (e.g., Position, Normal, Albedo)
     */
    void debugGBufferPass(const Shader& debugShader, int debugMode);

    int getNumAttachments();

private:
    /*
    * Viewport
    */
    int m_width;
    int m_height;
    unsigned int m_quadVAO;
    Camera* m_camera;

    /*
     * Initialize OpenGL state (depth testing, face culling, etc.)
     */
    void initOpenGLState();

    /*
    * Init mesh buffer for screen quad
    */
    void initScreenQuad();

    // List of framebuffers (in the future)
    std::unique_ptr<Framebuffer> m_gBuffer;

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
