#ifndef GEOMETRYPASS_H
#define GEOMETRYPASS_H

#include "renderpass.h"

class GeometryPass : public RenderPass {
public:
    // Updated constructor to accept instance counts
    explicit GeometryPass(const std::vector<Mesh*>& meshInstances)
        : m_meshInstances(meshInstances) {
        // Initialize the drawn flags vector to false for all instances
        m_instanceDrawnFlags.resize(meshInstances.size(), false);
    }

    void setup() override {
        std::string gBufferVertexPath = ASSET_DIR "shaders/core/deferred/gbuff.vs";
        std::string gBufferFragmentPath = ASSET_DIR "shaders/core/deferred/gbuff.fs";
        if (!m_gBufferShader.load(gBufferVertexPath, gBufferFragmentPath)) {
            std::cerr << "[Error] Renderer::Renderer: Failed to create gBufferShader!\n";
        }
    }

    void resetInstanceDrawFlags() {
        // Reset the drawn flags at the start of every frame
        std::fill(m_instanceDrawnFlags.begin(), m_instanceDrawnFlags.end(), false);
    }

    void execute(Renderer& renderer, entt::registry& registry) override {
        // Reset flags before starting a new frame
        resetInstanceDrawFlags();

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

        // Render instanced meshes
        registry.view<MeshInstance, ModelMatrix>().each([&](MeshInstance& instance, const ModelMatrix& modelMatrix) {
            size_t id = instance.id;
            if (id >= m_meshInstances.size()) {
                return;
            }

            if (!m_meshInstances[id]->material->isDeferred) {
                return;
            }

            // Check if this instance has already been drawn in this frame
            if (m_instanceDrawnFlags[id]) {
                return;
            }

            // TODO: this is where the logic doesnt add up
            const glm::mat4& instanceModelMatrix = modelMatrix.matrix;
            m_gBufferShader.setMat4("u_Model", instanceModelMatrix);
            m_meshInstances[id]->material->bind(&m_gBufferShader);

            // Draw and mark this instance as drawn
            renderer.drawInstanced(id);
            m_instanceDrawnFlags[id] = true;
        });

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
    std::vector<Mesh*> m_meshInstances;        // Mesh instances
    std::vector<bool> m_instanceDrawnFlags;    // Track which instances have been drawn
};

#endif // GEOMETRYPASS_H
