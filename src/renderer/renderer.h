#pragma once

#include "../engine.h"

#include "framebuffer.h"
#include "cubeMap.h"
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
    * Indirect Mesh Drawing
    */
    void initIndirectDrawBuffer(size_t maxDrawCommands);
    void addIndirectDrawCommand(const Mesh& mesh);
    void updateIndirectDrawBuffer();
    void drawMultiIndirect(bool indexed);
    void clearIndirectCommands();

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
    void resize(int width, int height);

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
    * Mesh preprocessing
    */
    ComputeShader m_heightCompute;
    void applyHeightMapCompute(Mesh* mesh);

    /*
    * Indirect draw buffers
    */
    struct DrawElementsIndirectCommand {
        GLuint count;         // Number of elements (indices) to draw
        GLuint instanceCount; // Number of instances to draw
        GLuint firstIndex;    // Base index within the index buffer
        GLuint baseVertex;    // Value added to each index
        GLuint baseInstance;  // Base instance for this draw command
    };

    struct DrawArraysIndirectCommand {
        GLuint count;         // Number of vertices to draw
        GLuint instanceCount; // Number of instances to draw
        GLuint first;         // Starting vertex index
        GLuint baseInstance;  // Base instance for this draw command
    };

    GLuint m_indirectBuffer = 0;
    GLuint m_indirectCount = 0;
    GLuint m_drawCountBuffer = 0;
    std::vector<DrawElementsIndirectCommand> m_indirectCommands;

    /*
     * Mesh buffer storage
     */
    struct MeshData {
        GLuint VAO, VBO, EBO;
        GLsizei indexCount;
        GLsizei vertexCount;
        GLuint instanceVBO = 0;

        GLuint instanceSSBO = 0;
        GLuint instanceBufferSize = 0;
        GLsizei instanceCount = 0;
    };

    std::vector<Material> m_materials;
    std::vector<MeshData> m_meshData;
    std::vector<MeshData> m_instanceMeshData;
};
