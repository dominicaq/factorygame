#ifndef LIGHTSYSTEM_H
#define LIGHTSYSTEM_H

#include "../light.h"
#include "../transform_components.h"
#include "config/settings.h"

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class LightSystem {
public:
    explicit LightSystem(config::GraphicsSettings& settings, Camera& activeCamera, entt::registry& registry);
    void updateShadowMatrices();

private:
    entt::registry& m_registry;
    Camera& m_activeCamera;

    config::GraphicsSettings& m_settings;

    // Light functions
    void calculateSpotMatrix(glm::mat4& matrix, const glm::vec3& position, const glm::vec3& direction, const Light& light);
    void updateDirectionalLightMatrices(LightSpaceMatrixArray& lightSpaceArray, const glm::vec3& direction);
    void updatePointLightMatrices(LightSpaceMatrixArray& lightSpaceCube, const glm::vec3& position, float radius);
};

#endif // LIGHTSYSTEM_H
