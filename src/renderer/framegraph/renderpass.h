#ifndef RENDERPASS_H
#define RENDERPASS_H

#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>
#include "entt/entt.hpp"
#include "../renderer.h"

class Renderer;
class FrameGraph;
class Mesh;

class RenderPass {
public:
    virtual ~RenderPass() = default;
    virtual void setup() = 0;
    virtual void execute(Renderer& renderer, entt::registry& registry) = 0;

    // Set the parent frame graph (called by FrameGraph when the pass is added)
    void setFrameGraph(FrameGraph* frameGraph) {
        m_frameGraph = frameGraph;
    }

protected:
    // Access to the parent frame graph
    FrameGraph* m_frameGraph = nullptr;

    // Helper to get the shared instance map
    const std::unordered_map<size_t, std::vector<glm::mat4>>& getInstanceMap() const;

    // Helper to get the mesh instances
    const std::vector<Mesh*>& getMeshInstances() const;
};

#endif // RENDERPASS_H