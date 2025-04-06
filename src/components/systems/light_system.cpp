#include "light_system.h"

LightSystem::LightSystem(config::GraphicsSettings& settings, entt::registry& registry) : m_registry(registry), m_settings(settings) {}

void LightSystem::updateShadowMatrices() {
    m_registry.view<Light, Position>().each([this](auto entity, Light& light, Position& position) {
        if (!light.castShadow || !light.isActive) {
            return; // Skip lights that don't need updates
        }

        switch (light.type) {
            case LightType::Point:
                if (m_registry.all_of<LightSpaceMatrixArray>(entity)) {
                    LightSpaceMatrixArray& lightSpaceCube = m_registry.get<LightSpaceMatrixArray>(entity);
                    updatePointLightMatrices(lightSpaceCube, position.position, light.point.radius);
                }
                break;

            case LightType::Directional:
                if (m_registry.all_of<LightSpaceMatrixArray>(entity)) {
                    // Handle directional lights with matrix arrays
                    LightSpaceMatrixArray& lightSpaceArray = m_registry.get<LightSpaceMatrixArray>(entity);
                    Rotation& rotationComponent = m_registry.get<Rotation>(entity);

                    // Forward is the -Z axis
                    glm::vec3 lightDirection = rotationComponent.quaternion * glm::vec3(0.0f, 0.0f, -1.0f);
                    updateDirectionalLightMatrices(lightSpaceArray, position.position, glm::normalize(lightDirection), light);
                }
                break;

            default:
                // Handle other light types and legacy single-matrix support
                if (m_registry.all_of<LightSpaceMatrix>(entity)) {
                    // Legacy single-matrix support
                    LightSpaceMatrix& lightSpaceComponent = m_registry.get<LightSpaceMatrix>(entity);
                    Rotation& rotationComponent = m_registry.get<Rotation>(entity);

                    // Forward is the -Z axis
                    glm::vec3 lightDirection = rotationComponent.quaternion * glm::vec3(0.0f, 0.0f, -1.0f);
                    lightSpaceComponent.matrix = calculateSpotMatrix(position.position, glm::normalize(lightDirection), light);
                }
                break;
        }
    });
}

void LightSystem::updatePointLightMatrices(LightSpaceMatrixArray& lightSpaceCube, const glm::vec3& position, float radius) {
    glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, 1.5f * radius);

    // Define the view-projection matrices for each face of the cubemap
    lightSpaceCube.matrices[0] = projection * glm::lookAt(position, position + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f,  1.0f,  0.0f)); // Right face (+X)
    lightSpaceCube.matrices[1] = projection * glm::lookAt(position, position + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f,  1.0f,  0.0f)); // Left face (-X)
    lightSpaceCube.matrices[2] = projection * glm::lookAt(position, position + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)); // Up face (+Y)
    lightSpaceCube.matrices[3] = projection * glm::lookAt(position, position + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)); // Down face (-Y)
    lightSpaceCube.matrices[4] = projection * glm::lookAt(position, position + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f,  1.0f,  0.0f)); // Front face (+Z)
    lightSpaceCube.matrices[5] = projection * glm::lookAt(position, position + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f,  1.0f,  0.0f)); // Back face (-Z)
}

void LightSystem::updateDirectionalLightMatrices(LightSpaceMatrixArray& lightSpaceArray, const glm::vec3& position, const glm::vec3& direction, const Light& light) {
    // Calculate right and up vectors for consistent orientation
    glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), direction));
    glm::vec3 up = glm::normalize(glm::cross(direction, right));

    float orthoSize = light.directional.shadowOrthoSize;

    // Cascade configuration
    int numCascades = m_settings.shadows.cascades.numCascades;

    // Create the cascade shadow maps
    for (int i = 0; i < numCascades; ++i) {
        float nearPlane = m_settings.shadows.cascades.cascadeNearPlanes[i];
        float farPlane = m_settings.shadows.cascades.cascadeFarPlanes[i];
        float sizeMultiplier = m_settings.shadows.cascades.cascadeSizeMultipliers[i];
        float centerPos = m_settings.shadows.cascades.cascadeCenterPositions[i];

        glm::mat4 projection = glm::ortho(
            -orthoSize * sizeMultiplier, orthoSize * sizeMultiplier,
            -orthoSize * sizeMultiplier, orthoSize * sizeMultiplier,
            nearPlane, farPlane
        );

        glm::vec3 lightPos = position - direction * centerPos;
        glm::mat4 view = glm::lookAt(lightPos, position, up);

        lightSpaceArray.matrices[i] = projection * view;
    }
}

glm::mat4 LightSystem::calculateSpotMatrix(const glm::vec3& position, const glm::vec3& direction, const Light& light) {
    glm::mat4 projection(1.0f);
    glm::mat4 view(1.0f);

    glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), direction));
    glm::vec3 up = glm::normalize(glm::cross(direction, right));

    float outerCutoff = glm::clamp(light.spot.outerCutoff, -1.0f, 1.0f);
    float fov = glm::acos(outerCutoff) * 2.0f;
    projection = glm::perspective(fov, 1.0f, 1.0f, light.spot.range);
    view = glm::lookAt(position, position + direction, up);

    return projection * view;
}
