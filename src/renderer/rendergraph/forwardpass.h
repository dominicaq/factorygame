#ifndef FORWARDPASS_H
#define FORWARDPASS_H

#include "renderpass.h"

class ForwardPass : public RenderPass {
public:
    explicit ForwardPass() = default;
    void setup() override {};
    void execute(Renderer& renderer, entt::registry& registry) override {
        // Get Resources
        Camera* camera = renderer.getCamera();
        const glm::mat4& viewMatrix = camera->getViewMatrix();

        // broken
        // auto& viewMatrixResource = registry.ctx().get<ViewMatrixResource>();
        // const glm::mat4& viewMatrix = viewMatrixResource.viewMatrix;

        // Enable depth testing for forward pass
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        auto viewMesh = registry.view<Mesh*, ModelMatrix>();
        for (auto entity : viewMesh) {
            const auto& mesh = registry.get<Mesh*>(entity);
            const auto& modelMatrix = registry.get<ModelMatrix>(entity);

            // Skip deferred rendering materials
            if (mesh->material->isDeferred) {
                continue;
            }

            Shader* shader = mesh->material->shader;
            shader->use();

            // Set transformation matrices
            shader->setMat4("u_View", viewMatrix);
            shader->setMat4("u_Projection", camera->getProjectionMatrix());
            shader->setMat4("u_Model", modelMatrix.matrix);

            mesh->material->bind();
            renderer.draw(mesh);
        }
    };
};

#endif // FORWARDPASS_H
