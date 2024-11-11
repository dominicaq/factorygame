#ifndef RENDERGRAPH_H
#define RENDERGRAPH_H

#include <memory>
#include <vector>
#include "renderpass.h"
#include "entt/entt.hpp"

class FrameGraph {
public:
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
    void executePasses(Renderer& renderer, entt::registry& registry) {
        for (auto& pass : m_renderPasses) {
            pass->execute(renderer, registry);
        }
    }

private:
    std::vector<std::unique_ptr<RenderPass>> m_renderPasses;
};

#endif // RENDERGRAPH_H
