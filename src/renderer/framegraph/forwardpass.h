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

        // Vector to store transparent objects for sorting
        std::vector<std::tuple<float, Mesh*, glm::mat4>> transparentObjects;
        glm::vec3 cameraPosition = camera->getPosition();

        // Single pass through the scene - render opaque immediately, queue transparent
        registry.view<Mesh*, ModelMatrix>().each([&](Mesh* mesh, const ModelMatrix& modelMatrix) {
            // Skip deferred materials
            if (mesh->material->isDeferred) {
                return;
            }

            // Process based on transparency
            if (mesh->material->albedoColor.a >= 1.0f) {
                // Render opaque objects immediately
                Shader* shader = mesh->material->shader;
                shader->use();

                // Set transformation matrices
                shader->setMat4("u_View", viewMatrix);
                shader->setMat4("u_Projection", camera->getProjectionMatrix());
                shader->setMat4("u_Model", modelMatrix.matrix);

                mesh->material->bind();
                renderer.draw(mesh);
            } else {
                // Queue transparent objects for later rendering
                glm::vec3 objectPosition = glm::vec3(modelMatrix.matrix[3]);
                float distanceToCamera = glm::length(cameraPosition - objectPosition);
                transparentObjects.emplace_back(distanceToCamera, mesh, modelMatrix.matrix);
            }
        });

        // If we have transparent objects to render
        if (!transparentObjects.empty()) {
            // Enable blending for transparent objects
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Disable depth writing but keep depth testing
            glDepthMask(GL_FALSE);

            // Sort transparent objects back-to-front (furthest first)
            std::sort(transparentObjects.begin(), transparentObjects.end(),
                [](const auto& a, const auto& b) {
                    return std::get<0>(a) > std::get<0>(b);
                });

            // Render sorted transparent objects
            for (const auto& [distance, mesh, modelMatrix] : transparentObjects) {
                Shader* shader = mesh->material->shader;
                shader->use();

                // Set transformation matrices
                shader->setMat4("u_View", viewMatrix);
                shader->setMat4("u_Projection", camera->getProjectionMatrix());
                shader->setMat4("u_Model", modelMatrix);

                mesh->material->bind();
                renderer.draw(mesh);
            }

            // Reset OpenGL state
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }
    }
};

#endif // FORWARDPASS_H
