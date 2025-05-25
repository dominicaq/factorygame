#ifndef RENDERPASS_H
#define RENDERPASS_H

#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>
#include "entt/entt.hpp"
#include "../renderBatch.h"
#include "../renderer.h"

class Renderer;
class FrameGraph;
struct Mesh;
class Scene;

class RenderPass {
public:
    virtual ~RenderPass() = default;
    virtual void setup() = 0;
    virtual void execute(Renderer& renderer, entt::registry& registry) = 0;

    // Set the scene reference (this will be called by FrameGraph)
    void setScene(Scene* scene) {
        m_scene = scene;
    }

protected:
    Scene* m_scene = nullptr;
};

#endif // RENDERPASS_H
