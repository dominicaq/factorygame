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

    void execute(Renderer& renderer, entt::registry& registry) override {
        // Get resources
        Framebuffer* gbuffer = renderer.getFramebuffer();
        Camera* camera = renderer.getCamera();
        const glm::mat4& viewMatrix = camera->getViewMatrix();

        // Bind G-buffer framebuffer
        gbuffer->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use Geometry Pass Shader
        m_gBufferShader.use();
        m_gBufferShader.setVec3("u_ViewPos", camera->getPosition());
        m_gBufferShader.setMat4("u_View", viewMatrix);
        m_gBufferShader.setMat4("u_Projection", camera->getProjectionMatrix());

        // Clear and build draw commands for indirect rendering
        renderer.clearDrawCommands();

        // Batch draw
        RenderBatch batch;
        registry.view<Mesh, ModelMatrix>().each([&](const Mesh& mesh, const ModelMatrix& modelMatrix) {
            if (mesh.material->isDeferred) {
                batch.addInstance(mesh, modelMatrix.matrix, mesh.material->uvScale);
            }
        });

        // Draw scene
        batch.prepare(renderer);
        batch.render(renderer);

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
};

#endif // GEOMETRYPASS_H
