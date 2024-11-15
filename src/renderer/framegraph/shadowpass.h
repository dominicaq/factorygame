#ifndef SHADOWPASS_H
#define SHADOWPASS_H

#include "renderpass.h"
#include "../framebuffer.h"
#include "../renderer.h"

class ShadowPass : public RenderPass {
public:
    explicit ShadowPass() = default;

    void setup() override;
    void execute(Renderer& renderer, entt::registry& registry) override;

private:
    Shader m_shadowShader;

    void renderSceneDepth(Renderer& renderer, entt::registry& registry);
};

#endif // SHADOWPASS_H
