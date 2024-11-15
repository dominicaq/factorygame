#ifndef SHADOWPASS_H
#define SHADOWPASS_H

#include "renderpass.h"
#include "../framebuffer.h"
#include "../renderer.h"

class ShadowPass : public RenderPass {
public:
    explicit ShadowPass(Renderer& renderer, unsigned int atlasSize = 4096, unsigned int tileSize = 1024);

    void setup() override;
    void execute(Renderer& renderer, entt::registry& registry) override;

private:
    unsigned int m_atlasSize;
    unsigned int m_tileSize;
    Shader m_shadowShader;

    void renderSceneDepth(Renderer& renderer, entt::registry& registry);
};

#endif // SHADOWPASS_H
