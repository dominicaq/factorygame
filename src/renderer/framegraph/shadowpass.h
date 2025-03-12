#ifndef SHADOWPASS_H
#define SHADOWPASS_H

#include "renderpass.h"
#include "../framebuffer.h"
#include "../renderer.h"

class ShadowPass : public RenderPass {
public:
    explicit ShadowPass(const std::vector<Mesh*>& meshInstances)
    : m_meshInstances(meshInstances) {}

    void setup() override;
    void execute(Renderer& renderer, entt::registry& registry) override;

private:
    Shader m_shadowShader;
    std::vector<Mesh*> m_meshInstances;
    std::unordered_map<size_t, std::vector<glm::mat4>> m_instanceMap;
    void prepareInstanceMap(entt::registry& registry);
    void renderSceneDepth(Renderer& renderer, entt::registry& registry);
};

#endif // SHADOWPASS_H
