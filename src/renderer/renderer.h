#ifndef RENDERER_H
#define RENDERER_H

#include "../engine.h"

#include "framebuffer.h"
#include "cube_map.h"
#include "config/settings.h"

#include <glad/glad.h>
#include <memory>
#include <vector>

/*
 * The Renderer class is responsible for handling OpenGL rendering,
 * including G-buffer management for deferred rendering and mesh rendering.
 */
class Renderer {
public:
    Renderer(config::GraphicsSettings settings, Camera* camera);
    ~Renderer();

    config::GraphicsSettings config;
    bool applySettings(const config::GraphicsSettings& settings);

    /*
    * Getters
    */
    std::pair<int, int> getScreenDimensions() const {
        return {m_width, m_height};
    }
    Framebuffer* getFramebuffer() const { return m_gBuffer.get(); }
    Camera* getCamera() const { return m_camera; }

    /*
    * Instancing
    */
    void drawInstanced(size_t instanceID, bool wireframe = false);
    void updateInstanceBuffer(size_t instanceID, const std::vector<glm::mat4>& modelMatrices);
    void setupInstanceAttributes(size_t instanceID, const std::vector<glm::mat4>& modelMatrices);
    void deleteInstanceBuffer(size_t instanceID);

    /*
     * Initialize and manage mesh buffers
     */
    void draw(const Mesh* mesh);
    void initMeshBuffers(Mesh* mesh, bool isStatic = true, size_t instanceID = SIZE_MAX);
    void deleteMeshBuffer(const Mesh* mesh);

    /*
     * Quad rendering (used for post-processing, G-buffer display, etc.)
     */
    void drawScreenQuad();

    /*
     * Recreate G-buffer with new dimensions
     */
    void resizeGBuffer(int width, int height);

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
    std::unique_ptr<Framebuffer> m_shadowAtlas;

    /*
     * Mesh buffer storage
     */
    struct MeshData {
        unsigned int VAO, VBO, EBO;
        GLsizei indexCount;
        GLsizei vertexCount;

        GLsizei instanceCount;
    };

    std::vector<MeshData> m_meshData;
    std::vector<MeshData> m_instanceMeshData;
};

#endif // RENDERER_H
