#pragma once

#include "../../config/settings.h"
#include "../camera.h"
#include "../light.h"
#include "../transform.h"
#include "config/settings.h"

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class LightSystem {
public:
    explicit LightSystem(config::GraphicsSettings& settings, entt::registry& registry);
    void updateShadowMatrices(const Camera& activeCamera);

private:
    entt::registry& m_registry;
    config::GraphicsSettings& m_settings;

    // Light functions
    void calculateSpotMatrix(glm::mat4& matrix, const glm::vec3& position, const glm::vec3& direction, const Light& light);
    void updateDirectionalLightMatrices(const Camera& activeCamera, LightSpaceMatrixArray& lightSpaceArray, const glm::vec3& lightDir);
    void updatePointLightMatrices(LightSpaceMatrixArray& lightSpaceCube, const glm::vec3& position, float radius);
};
