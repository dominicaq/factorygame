#include "light_system.h"

LightSystem::LightSystem(entt::registry& registry) : m_registry(registry) {}

void LightSystem::updateShadowMatrices() {
    m_registry.view<Light, Position>().each([this](auto entity, Light& light, Position& position) {
        if (!light.castsShadows || !light.isActive) {
            return; // Skip lights that don't cast shadows
        }

        if (light.type == LightType::Point && m_registry.all_of<LightSpaceMatrixCube>(entity)) {
            LightSpaceMatrixCube& lightSpaceCube = m_registry.get<LightSpaceMatrixCube>(entity);
            updatePointLightMatrices(lightSpaceCube, position.position, light.point.radius);

        } else if (m_registry.all_of<LightSpaceMatrix, EulerAngles>(entity)) {
            LightSpaceMatrix& lightSpaceComponent = m_registry.get<LightSpaceMatrix>(entity);
            Rotation& rotationComponent = m_registry.get<Rotation>(entity);

            // Forward is the +Z axis, so we need to reverse the dir
            glm::vec3 lightDirection = rotationComponent.quaternion * glm::vec3(0.0f, 0.0f, -1.0f);
            lightSpaceComponent.matrix = calculateLightSpaceMatrix(
                position.position,
                glm::normalize(lightDirection),
                light
            );
        }
    });
}

void LightSystem::updatePointLightMatrices(LightSpaceMatrixCube& lightSpaceCube, const glm::vec3& position, float radius) {
    glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, 3 * radius);

    // Define the view-projection matrices for each face of the cubemap
    lightSpaceCube.matrices[0] = projection * glm::lookAt(position, position + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f,  1.0f,  0.0f)); // Right face (+X)
    lightSpaceCube.matrices[1] = projection * glm::lookAt(position, position + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f,  1.0f,  0.0f)); // Left face (-X)
    lightSpaceCube.matrices[2] = projection * glm::lookAt(position, position + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)); // Up face (+Y)
    lightSpaceCube.matrices[3] = projection * glm::lookAt(position, position + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)); // Down face (-Y)
    lightSpaceCube.matrices[4] = projection * glm::lookAt(position, position + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f,  1.0f,  0.0f)); // Front face (+Z)
    lightSpaceCube.matrices[5] = projection * glm::lookAt(position, position + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f,  1.0f,  0.0f)); // Back face (-Z)
}

glm::mat4 LightSystem::calculateLightSpaceMatrix(const glm::vec3& position, const glm::vec3& direction, const Light& light) {
    glm::mat4 projection(1.0f);
    glm::mat4 view(1.0f);

    glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), direction));
    glm::vec3 up = glm::normalize(glm::cross(direction, right));

    switch (light.type) {
        case LightType::Directional: {
            float orthoSize = light.directional.shadowOrthoSize;
            projection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 1.0f, 100.0f);
            glm::vec3 lightPos = position - direction * 50.0f;
            view = glm::lookAt(lightPos, position, up);
            break;
        }
        case LightType::Spot: {
            float outerCutoff = glm::clamp(light.spot.outerCutoff, -1.0f, 1.0f);
            float fov = glm::acos(outerCutoff) * 2.0f;
            projection = glm::perspective(fov, 1.0f, 1.0f, light.spot.range);
            view = glm::lookAt(position, position + direction, up);
            break;
        }
        default:
            // Point lights are handled differently
            break;
    }

    return projection * view;
}
