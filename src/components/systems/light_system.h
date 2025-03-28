#ifndef LIGHTSYSTEM_H
#define LIGHTSYSTEM_H

#include "../light.h"
#include "../transform_components.h"

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class LightSystem {
public:
    explicit LightSystem(entt::registry& registry);
    void updateShadowMatrices();

private:
    entt::registry& m_registry;

    glm::mat4 calculateLightSpaceMatrix(const glm::vec3& position, const glm::vec3& direction, const Light& light);
    void updatePointLightMatrices(LightSpaceMatrixCube& lightSpaceCube, const glm::vec3& position, float radius);
};

#endif // LIGHTSYSTEM_H
