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
        m_gBufferShader.setMat4("u_View", viewMatrix);
        m_gBufferShader.setMat4("u_Projection", camera->getProjectionMatrix());

        // Render regular meshes
        registry.view<Mesh*, ModelMatrix>().each([&](Mesh* mesh, const ModelMatrix& modelMatrix) {
            if (!mesh->material->isDeferred) {
                return;
            }

            m_gBufferShader.setMat4("u_Model", modelMatrix.matrix);
            mesh->material->bind(&m_gBufferShader);
            renderer.draw(mesh);
        });

        // Get scene data for rendering
        const auto& instanceMap = m_scene->getInstanceMap();
        const auto& meshInstances = m_scene->getMeshInstances();

        // Render all instances using the shared instance map
        for (const auto& [meshId, matrices] : instanceMap) {
            if (matrices.empty() || meshId >= meshInstances.size() ||
                !meshInstances[meshId]->material->isDeferred) {
                continue;
            }

            renderer.updateInstanceBuffer(meshId, matrices);

            // Set the first matrix as the model uniform (for gl_InstanceID == 0)
            m_gBufferShader.setMat4("u_Model", matrices[0]);
            meshInstances[meshId]->material->bind(&m_gBufferShader);
            renderer.drawInstanced(meshId);
        }

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
