#ifndef SHADOWPASS_H
#define SHADOWPASS_H

#include "renderpass.h"
#include "../framebuffer.h"
#include "../renderer.h"
#include <unordered_map>
#include <entt/entity/registry.hpp>

#define SHADOW_RESOLUTION 1024

class ShadowPass : public RenderPass {
public:
    explicit ShadowPass() = default;
    ~ShadowPass();

    void setup() override;
    void execute(Renderer& renderer, entt::registry& registry) override;
    void cleanupLightResources(entt::entity lightEntity);

private:
    Shader m_shadowShader;
    unsigned int m_shadowFrameBuffer = 0;

    // Caches for storing shadow textures per light entity
    std::unordered_map<entt::entity, unsigned int> m_lightShadowMapMap;
    std::unordered_map<entt::entity, unsigned int> m_lightCubemapMap;

    // Helper methods
    void renderSceneDepth(Renderer& renderer, entt::registry& registry);

    // Texture creation helpers
    unsigned int createShadowMap();
    unsigned int createCubeMapAtlas();
};

#endif // SHADOWPASS_H