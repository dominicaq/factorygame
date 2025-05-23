#ifndef FRAMEGRAPH_H
#define FRAMEGRAPH_H

#include <memory>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include "renderpass.h"
#include "entt/entt.hpp"

// Forward declaration
struct Mesh;
class Scene;

class FrameGraph {
public:
    explicit FrameGraph(Scene& scene) : m_activeScene(scene) {}

    // Add a render pass to the frame graph
    void addRenderPass(std::unique_ptr<RenderPass> pass) {
        pass->setScene(&m_activeScene);
        m_renderPasses.push_back(std::move(pass));
    }

    // Set up all render passes in the frame graph
    void setupPasses() {
        for (auto& pass : m_renderPasses) {
            pass->setup();
        }
    }

    // Execute all render passes, passing in the renderer and registry
    void executePasses(Renderer& renderer, entt::registry& registry) {
        // Clear the default framebuffer at the start of each frame
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (auto& pass : m_renderPasses) {
            pass->execute(renderer, registry);
        }
    }

private:
    Scene& m_activeScene;
    std::vector<std::unique_ptr<RenderPass>> m_renderPasses;
};

#endif // FRAMEGRAPH_H
