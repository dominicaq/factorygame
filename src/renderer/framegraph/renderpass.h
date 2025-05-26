#pragma once

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
class Camera;

class RenderPass {
public:
    virtual ~RenderPass() = default;
    virtual void setup() = 0;
    virtual void execute(entt::registry& registry, Camera& camera, Renderer& renderer) = 0;

    // Set the scene reference (this will be called by FrameGraph)
    void setScene(Scene* scene) {
        m_scene = scene;
    }

protected:
    Scene* m_scene = nullptr;
};
