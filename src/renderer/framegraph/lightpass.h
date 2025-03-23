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
        glm::vec3 position; float _padding1;  // 16 bytes
        glm::vec3 direction; float _padding2; // 16 bytes
        glm::vec3 color; float intensity;  // 16 bytes
        float innerCutoff; float outerCutoff; int castShadow; int shadowMapIndex; // 16 bytes
        int lightMatrixIndex; int _padding3; int _padding4; int _padding5; // 16 bytes
    };

    // Light data
    std::vector<PointLightSSBO> m_pointData;
    std::vector<glm::mat4> m_lightMatrixData;
    std::vector<GLuint> m_shadowMapHandles;

    // SSBO handles
    GLuint m_pointSSBO;
    GLuint m_lightMatrixSSBO;

    // Light pass frame shader
    Shader m_lightPassShader;

    unsigned int m_skyboxTexture;
};

#endif // LIGHTPASS_H
