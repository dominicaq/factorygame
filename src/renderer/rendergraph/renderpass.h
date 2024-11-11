#ifndef RENDERPASS_H
#define RENDERPASS_H

#include "entt/entt.hpp"
#include "../renderer.h"

class RenderPass {
public:
    virtual ~RenderPass() = default;
    virtual void setup() = 0;
    virtual void execute(Renderer& renderer, entt::registry& registry) = 0;
};

#endif // RENDERPASS_H
