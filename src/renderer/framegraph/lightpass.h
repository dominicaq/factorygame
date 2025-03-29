#ifndef LIGHTPASS_H
#define LIGHTPASS_H

#include "renderpass.h"

class LightPass : public RenderPass {
public:
    explicit LightPass() = default;
    void setup() override;
    void execute(Renderer& renderer, entt::registry& registry) override;

private:
    struct PointLightSSBO {
        glm::vec3 position; float radius;  // 16 bytes
        glm::vec3 color; float intensity;  // 16 bytes
        int castShadow; int shadowMapIndex; int lightMatrixIndex; int _padding; // 16 bytes
    };

    struct DirectionalLightSSBO {
        glm::vec3 direction; float shadowOrthoSize;  // 16 bytes
        glm::vec3 color; float intensity;  // 16 bytes
        int castShadow; int shadowMapIndex; int lightMatrixIndex; int _padding; // 16 bytes
    };

    struct SpotLightSSBO {
        glm::vec3 position; float innerCutoff;  // 16 bytes
        glm::vec3 direction; float outerCutoff; // 16 bytes
        glm::vec3 color; float intensity;       // 16 bytes
        float range; int castShadow; int shadowMapIndex; int lightMatrixIndex; // 16 bytes
    };

    // Light data
    std::vector<PointLightSSBO> m_pointData;
    std::vector<SpotLightSSBO> m_spotData;
    std::vector<glm::mat4> m_lightMatrixData;
    std::vector<GLuint> m_shadowMapHandles;

    // SSBO handles
    GLuint m_spotSSBO;
    GLuint m_pointSSBO;
    GLuint m_lightMatrixSSBO;

    // Light pass frame shader
    Shader m_lightPassShader;

    unsigned int m_skyboxTexture;
};

#endif // LIGHTPASS_H
