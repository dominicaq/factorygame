#ifndef LIGHTPASS_H
#define LIGHTPASS_H

#include "renderpass.h"

class LightPass : public RenderPass {
public:
    explicit LightPass() = default;
    void setup() override;
    void execute(Renderer& renderer, entt::registry& registry) override;

private:
    // Note: structure must be aligned 16 bytes pad if needed.
    struct LightSSBO {
        // 16 bytes
        glm::vec3 position; int _pad1; // Padding to align vec3 to 16 bytes
        // 16 bytes
        glm::vec3 color; int _pad2;
        // 16 bytes
        float radius;
        float intensity;
        unsigned int depthHandle;
        int isPointLight;
    };

    GLuint m_lightSSBO;
    Shader m_lightPassShader;
};

#endif // LIGHTPASS_H
