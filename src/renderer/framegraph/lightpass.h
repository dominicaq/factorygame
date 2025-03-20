#ifndef LIGHTPASS_H
#define LIGHTPASS_H

#include "renderpass.h"

class LightPass : public RenderPass {
public:
    explicit LightPass() = default;
    void setup() override;
    void execute(Renderer& renderer, entt::registry& registry) override;

private:
    struct LightSSBO {
        glm::vec3 position; float radius;                  // 16 bytes
        glm::vec3 color; float intensity;                  // 16 bytes
        int isPointLight; int castShadow; int pad_2; int pad_3; // 16 bytes
        glm::mat4 lightSpaceMatrices[6]; // 64 * 6 bytes
    };

    Shader m_lightPassShader;
    GLuint m_lightSSBO;
};

#endif // LIGHTPASS_H
