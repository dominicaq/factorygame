#ifndef GEOMETRYPASS_H
#define GEOMETRYPASS_H

#include "renderpass.h"

class GeometryPass : public RenderPass {
public:
    // Updated constructor to accept instance counts
    explicit GeometryPass(const std::vector<Mesh*>& meshInstances)
        : m_meshInstances(meshInstances) {}

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


        // First, collect all instances by mesh ID
        std::unordered_map<size_t, std::vector<glm::mat4>> instancesByMeshId;
        registry.view<MeshInstance, ModelMatrix>().each([&](MeshInstance& instance, const ModelMatrix& modelMatrix) {
            size_t id = instance.id;
            if (id >= m_meshInstances.size() || !m_meshInstances[id]->material->isDeferred) {
                return;
            }
            instancesByMeshId[id].push_back(modelMatrix.matrix);
        });

        // Now draw each unique mesh just once, with all its instances
        for (const auto& [meshId, matrices] : instancesByMeshId) {
            if (matrices.empty()) {
                continue;
            }

            // Update buffer with instance matrices for this mesh
            renderer.updateInstanceBuffer(meshId, matrices);

            m_meshInstances[meshId]->material->bind(&m_gBufferShader);

            // Set the first matrix as the model uniform (for gl_InstanceID == 0)
            // This ensures the first instance is drawn correctly
            m_gBufferShader.setMat4("u_Model", matrices[0]);
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
    std::vector<Mesh*> m_meshInstances;
};

#endif // GEOMETRYPASS_H
