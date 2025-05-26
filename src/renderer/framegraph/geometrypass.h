#ifndef GEOMETRYPASS_H
#define GEOMETRYPASS_H

#include "renderpass.h"

class GeometryPass : public RenderPass {
public:
    // Updated constructor to accept instance counts
    explicit GeometryPass() = default;

    void setup() override {
        std::string gBufferVertexPath = ASSET_DIR "shaders/core/deferred/gbuff.vs";
        std::string gBufferFragmentPath = ASSET_DIR "shaders/core/deferred/gbuff.fs";
        if (!m_gBufferShader.load(gBufferVertexPath, gBufferFragmentPath)) {
            std::cerr << "[Error] Renderer::Renderer: Failed to create gBufferShader!\n";
        }
    }

    void execute(entt::registry& registry, Camera& camera, Renderer& renderer) override {
        // Get resources
        Framebuffer* gbuffer = renderer.getFramebuffer();

        // Bind G-buffer framebuffer
        gbuffer->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use Geometry Pass Shader
        m_gBufferShader.use();
        m_gBufferShader.setVec3("u_ViewPos", camera.getPosition());
        m_gBufferShader.setMat4("u_View", camera.getViewMatrix());
        m_gBufferShader.setMat4("u_Projection", camera.getProjectionMatrix());

        // Clear and build draw commands for indirect rendering
        m_geometryBatch.clear();

        // Batch draw
        auto& view = registry.view<Mesh, ModelMatrix>();
        for (const auto& entity : view) {
            const Mesh& mesh = view.get<Mesh>(entity);
            const ModelMatrix& modelMatrix = view.get<ModelMatrix>(entity);

            m_geometryBatch.addInstance(RenderInstance(mesh, modelMatrix.matrix));
        }
        MaterialManager::getInstance().bindMaterialBuffer(1);

        // Draw scene
        m_geometryBatch.prepare(renderer);
        m_geometryBatch.render(renderer);

        std::pair<int, int> dimensions = renderer.getScreenDimensions();
        int width = dimensions.first;
        int height = dimensions.second;

        // Copy depth buffer from G-buffer to default framebuffer
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);  // Default framebuffer (the screen)
        glBlitFramebuffer(
            0, 0, width, height,
            0, 0, width, height,
            GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        gbuffer->unbind();
    }

private:
    Shader m_gBufferShader;
    RenderBatch m_geometryBatch;
};

#endif // GEOMETRYPASS_H
