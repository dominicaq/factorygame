#ifndef GEOMETRYPASS_H
#define GEOMETRYPASS_H

#include "renderpass.h"

class GeometryPass : public RenderPass {
public:
    explicit GeometryPass() = default;

    void setup() override {
        std::string gBufferVertexPath = ASSET_DIR "shaders/core/deferred/gbuff.vs";
        std::string gBufferFragmentPath = ASSET_DIR "shaders/core/deferred/gbuff.fs";
        if (!m_gBufferShader.load(gBufferVertexPath, gBufferFragmentPath)) {
            std::cerr << "[Error] Renderer::Renderer: Failed to create gBufferShader!\n";
        }
    };

    void execute(Renderer& renderer, entt::registry& registry) override {
        // Get resources
        Framebuffer* gbuffer = renderer.getFramebuffer();
        Camera* camera = renderer.getCamera();
        const glm::mat4& viewMatrix = camera->getViewMatrix();

        // broken code
        // auto& viewMatrixResource = registry.ctx().get<ViewMatrixResource>();
        // end of broken code

        // Bind G-buffer framebuffer
        gbuffer->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use Geometry Pass Shader
        m_gBufferShader.use();
        m_gBufferShader.setMat4("u_View", viewMatrix);
        m_gBufferShader.setMat4("u_Projection", camera->getProjectionMatrix());

        // Loop through all entities with Mesh and ModelMatrix components
        // This is bad, but for now query for meshes in the individual passes
        auto viewMesh = registry.view<Mesh*, ModelMatrix>();
        for (auto entity : viewMesh) {
            const auto& mesh = registry.get<Mesh*>(entity);
            // Skip forward rendering materials
            if (!mesh->material->isDeferred) {
                continue;
            }

            const auto& modelMatrix = registry.get<ModelMatrix>(entity);
            m_gBufferShader.setMat4("u_Model", modelMatrix.matrix);

            mesh->material->bind(&m_gBufferShader);
            renderer.draw(mesh);
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
    };

private:
    Shader m_gBufferShader;
};

#endif // GEOMETRYPASS_H
