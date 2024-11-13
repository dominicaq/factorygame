#include "light_system.h"

LightSystem::LightSystem(entt::registry& registry) : m_registry(registry) {}

void LightSystem::updateShadowMatrices() {
    auto lightView = m_registry.view<Light, Position>();

    for (auto entity : lightView) {
        auto& light = lightView.get<Light>(entity);

        if (!light.castsShadows) {
            continue;
        }

        const auto& position = lightView.get<Position>(entity).position;

        if (light.type == LightType::Point) {
            auto& lightSpaceCube = m_registry.get<LightSpaceMatrixCube>(entity);
            updatePointLightMatrices(lightSpaceCube, position, light.radius);
        } else {
            auto& lightMatrix = m_registry.get<LightSpaceMatrix>(entity).matrix;
            const auto& eulerAngles = m_registry.get<EulerAngles>(entity).euler;

            glm::vec3 lightDirection;
            float pitch = glm::radians(eulerAngles.x);
            float yaw = glm::radians(eulerAngles.y);

            // Get forward direction
            lightDirection.x = cos(pitch) * sin(yaw);
            lightDirection.y = sin(pitch);
            lightDirection.z = cos(pitch) * cos(yaw);
            lightDirection = glm::normalize(lightDirection);

            lightMatrix = calculateLightSpaceMatrix(position, lightDirection, light.type, light.radius);
        }
    }
}

void LightSystem::updatePointLightMatrices(LightSpaceMatrixCube& lightSpaceCube, const glm::vec3& position, float radius) {
    glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, radius);

    // Define the view-projection matrices for each face of the cubemap
    lightSpaceCube.matrices[0] = projection * glm::lookAt(position, position + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)); // Right face (+X)
    lightSpaceCube.matrices[1] = projection * glm::lookAt(position, position + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)); // Left face (-X)
    lightSpaceCube.matrices[2] = projection * glm::lookAt(position, position + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)); // Up face (+Y)
    lightSpaceCube.matrices[3] = projection * glm::lookAt(position, position + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)); // Down face (-Y)
    lightSpaceCube.matrices[4] = projection * glm::lookAt(position, position + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)); // Front face (+Z)
    lightSpaceCube.matrices[5] = projection * glm::lookAt(position, position + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)); // Back face (-Z)
}

glm::mat4 LightSystem::calculateLightSpaceMatrix(const glm::vec3& position, const glm::vec3& direction, LightType type, float radius) {
    glm::mat4 projection(1.0f);
    glm::mat4 view(1.0f);

    switch (type) {
        case LightType::Directional: {
            float orthoSize = 20.0f;
            projection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 1.0f, 100.0f);
            view = glm::lookAt(position - direction * 50.0f, position, glm::vec3(0.0f, 1.0f, 0.0f));
            break;
        }
        case LightType::Spotlight: {
            projection = glm::perspective(glm::radians(45.0f), 1.0f, 1.0f, radius);
            view = glm::lookAt(position, position + direction, glm::vec3(0.0f, 1.0f, 0.0f));
            break;
        }
        default:
            break;
    }

    return projection * view;
}
