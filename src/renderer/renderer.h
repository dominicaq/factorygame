#pragma once

#include "../engine.h"
#include "renderBatch.h"
#include "framebuffer.h"
#include "cubeMap.h"
#include "config/settings.h"

#include <glad/glad.h>
#include <memory>
#include <vector>
#include <map>

// Forward declarations
struct IndirectDrawCommand;
struct DrawElementsIndirectCommand;
struct DrawArraysIndirectCommand;

/*
 * The Renderer class is responsible for handling OpenGL rendering,
 * including G-buffer management for deferred rendering and mesh rendering.
 */
class Renderer {
public:
    Renderer(config::GraphicsSettings settings);
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
    int getNumAttachments();

    /*
     * Core Rendering Interface - What the renderer should focus on
     */
    void executeIndirectDraw(const std::vector<IndirectDrawCommand>& commands, GLuint indirectBuffer);

    /*
     * Direct mesh drawing (for your current render loop)
     */
    void drawScreenQuad();

    /*
     * Viewport and framebuffer management
     */
    void resize(int width, int height);
    void setCameraTarget(Camera* camera);
    // ============================================================================
    // MESH MANAGEMENT SECTION - Keep this minimal for now
    // ============================================================================

    /*
     * Mesh buffer management - moved from your original code
     */
    Mesh initMeshBuffers(std::unique_ptr<RawMeshData>& rawData, bool isStatic = true);
    void deleteMeshBuffer(const Mesh& mesh);

    /*
     * Query functions for indirect rendering
     */
    GLuint getMeshVAO(size_t meshId) const;
    bool hasMeshIndices(size_t meshId) const;
    GLsizei getMeshIndexCount(size_t meshId) const;
    GLsizei getMeshVertexCount(size_t meshId) const;

private:
    /*
     * Core Renderer State
     */
    int m_width;
    int m_height;
    Camera* m_targetCamera;
    std::unique_ptr<Framebuffer> m_gBuffer;

    // Screen quad for deferred rendering
    GLuint m_quadVAO = 0;

    /*
     * Mesh data storage
     */
    struct MeshData {
        GLuint VAO = 0;
        GLuint VBO = 0;
        GLuint EBO = 0;
        uint32_t indexCount = 0;
        uint32_t vertexCount = 0;
    };
    std::vector<MeshData> m_meshData;

    /*
    * Material storage
    */

    /*
     * Initialization
     */
    void initOpenGLState();
    void initScreenQuad();
    void cleanup();
};
