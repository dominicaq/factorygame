#ifndef SHADOWPASS_H
#define SHADOWPASS_H

#include "renderpass.h"
#include "../framebuffer.h"
#include "../renderer.h"
#include <unordered_map>
#include <entt/entity/registry.hpp>

class ShadowPass : public RenderPass {
public:
    explicit ShadowPass() = default;
    ~ShadowPass();

    void setup() override;
    void execute(Renderer& renderer, entt::registry& registry) override;
    void cleanupLightResources(entt::entity lightEntity);

private:
    Shader m_shadowShader;
    Framebuffer* m_shadowFrameBuffer;

    // Caches for storing shadow textures per light entity
    std::unordered_map<entt::entity, unsigned int> m_lightShadowMapMap;
    std::unordered_map<entt::entity, unsigned int> m_lightArrayMap;

    // Helper methods
    void renderSceneDepth(Renderer& renderer, entt::registry& registry);

    // Texture creation helpers
    unsigned int createShadowMap(int shadowRes);
    unsigned int createLightStrip(int shadowRes, int numFaces);
};

#endif // SHADOWPASS_H
