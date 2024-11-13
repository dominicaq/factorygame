#ifndef SHADOWPASS_H
#define SHADOWPASS_H

#include "renderpass.h"
#include "../framebuffer.h"

class ShadowPass : public RenderPass {
public:
    explicit ShadowPass() = default;

    void setup() override {};
    void execute(Renderer& renderer, entt::registry& registry) override {};
};

#endif
