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
    explicit FrameGraph(Scene& scene) {}

    // Add a render pass to the frame graph
    void addRenderPass(std::unique_ptr<RenderPass> pass) {
        m_renderPasses.push_back(std::move(pass));
    }

    // Set up all render passes in the frame graph
    void setupPasses() {
        for (auto& pass : m_renderPasses) {
            pass->setup();
        }
    }

    // Execute all render passes, passing in the renderer and registry
    void executePasses(entt::registry& registry, Camera& camera, Renderer& renderer) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (auto& pass : m_renderPasses) {
            pass->execute(registry, camera, renderer);
        }
    }

private:
    std::vector<std::unique_ptr<RenderPass>> m_renderPasses;
};

#endif // FRAMEGRAPH_H
