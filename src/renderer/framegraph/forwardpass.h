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

        registry.view<Mesh*, ModelMatrix>().each([&](Mesh* mesh, const ModelMatrix& modelMatrix) {
            // Skip deferred rendering materials
            if (mesh->material->isDeferred) {
                return;
            }

            Shader* shader = mesh->material->shader;
            shader->use();

            // Set transformation matrices
            shader->setMat4("u_View", viewMatrix);
            shader->setMat4("u_Projection", camera->getProjectionMatrix());
            shader->setMat4("u_Model", modelMatrix.matrix);

            mesh->material->bind();
            renderer.draw(mesh);
        });
    }
};

#endif // FORWARDPASS_H
