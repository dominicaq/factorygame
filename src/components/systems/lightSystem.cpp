#include "lightSystem.h"

LightSystem::LightSystem(config::GraphicsSettings& settings, Camera& activeCamera, entt::registry& registry) : m_registry(registry), m_activeCamera{activeCamera}, m_settings(settings) {}

void LightSystem::updateShadowMatrices() {
    m_registry.view<Light, Position>().each([this](auto entity, Light& light, Position& position) {
        if (!light.castShadow || !light.isActive) {
            return; // Skip lights that don't need updates
        }

        switch (light.type) {
            case LightType::Point: {
                if (!m_registry.all_of<LightSpaceMatrixArray>(entity)) {
                    return;
                }

                LightSpaceMatrixArray& lightSpaceCube = m_registry.get<LightSpaceMatrixArray>(entity);
                updatePointLightMatrices(lightSpaceCube, position.position, light.point.radius);

                break;
            } case LightType::Directional: {
                if (!m_registry.all_of<LightSpaceMatrixArray>(entity)) {
                    return;
                }

                // Handle directional lights with matrix arrays
                LightSpaceMatrixArray& lightSpaceArray = m_registry.get<LightSpaceMatrixArray>(entity);
                Rotation& rotationComponent = m_registry.get<Rotation>(entity);

                // Forward is the -Z axis
                glm::vec3 lightDirection = rotationComponent.quaternion * glm::vec3(0.0f, 0.0f, -1.0f);
                updateDirectionalLightMatrices(lightSpaceArray, glm::normalize(lightDirection));

                break;
            } default: {
                if (!m_registry.all_of<LightSpaceMatrix>(entity)) {
                    return;
                }
                // Single-matrix support
                LightSpaceMatrix& lightSpace = m_registry.get<LightSpaceMatrix>(entity);
                Rotation& rotationComponent = m_registry.get<Rotation>(entity);

                // Forward is the -Z axis
                glm::vec3 lightDirection = rotationComponent.quaternion * glm::vec3(0.0f, 0.0f, -1.0f);
                calculateSpotMatrix(lightSpace.matrix, position.position, glm::normalize(lightDirection), light);

                break;
            }
        }
    });
}

void LightSystem::calculateSpotMatrix(glm::mat4& matrix,
    const glm::vec3& position,
    const glm::vec3& direction,
    const Light& light)
{
    glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), direction));
    glm::vec3 up = glm::normalize(glm::cross(direction, right));


    float outerCutoff = glm::clamp(light.spot.outerCutoff, -1.0f, 1.0f);
    float fov = glm::acos(outerCutoff) * 2.0f;

    glm::mat4 projection(1.0f);
    glm::mat4 view(1.0f);
    projection = glm::perspective(fov, 1.0f, 1.0f, light.spot.range);
    view = glm::lookAt(position, position + direction, up);

    matrix = projection * view;
}

void LightSystem::updatePointLightMatrices(LightSpaceMatrixArray& lightSpaceCube, const glm::vec3& position, float radius) {
    glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 2.0f * radius);

    // Define the view-projection matrices for each face of the cubemap
    lightSpaceCube.matrices[0] = projection * glm::lookAt(position, position + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f,  1.0f,  0.0f)); // Right face (+X)
    lightSpaceCube.matrices[1] = projection * glm::lookAt(position, position + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f,  1.0f,  0.0f)); // Left face (-X)
    lightSpaceCube.matrices[2] = projection * glm::lookAt(position, position + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)); // Up face (+Y)
    lightSpaceCube.matrices[3] = projection * glm::lookAt(position, position + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)); // Down face (-Y)
    lightSpaceCube.matrices[4] = projection * glm::lookAt(position, position + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f,  1.0f,  0.0f)); // Front face (+Z)
    lightSpaceCube.matrices[5] = projection * glm::lookAt(position, position + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f,  1.0f,  0.0f)); // Back face (-Z)
}

void LightSystem::updateDirectionalLightMatrices(LightSpaceMatrixArray& lightSpaceArray, const glm::vec3& lightDir) {
    // Calculate the splits
    int numCascades = m_settings.shadows.cascades.numCascades;
    std::vector<float> cascadeSplits(numCascades);

    float nearClip = m_activeCamera.getNearPlane();
    float farClip = m_activeCamera.getFarPlane();
    float clipRange = farClip - nearClip;

    float minZ = nearClip;
    float maxZ = nearClip + clipRange;

    float range = maxZ - minZ;
    float ratio = maxZ / minZ;

    for (int i = 0; i < numCascades; ++i) {
        float p = (i + 1) / static_cast<float>(numCascades);
        float log = minZ * std::pow(ratio, p);
        float uniform = minZ + range * p;
        float d = m_settings.shadows.cascades.cascadeSplitLambda * (log - uniform) + uniform;
        cascadeSplits[i] = (d - nearClip) / clipRange;
    }

    // Calculate the light view matrix for each split
    glm::mat4 projection = m_activeCamera.getProjectionMatrix();
    glm::mat4 view = m_activeCamera.getViewMatrix();
    glm::mat4 invCam = glm::inverse(projection * view);

    float lastSplitDist = 0.0f;
    for (size_t i = 0; i < numCascades; i++) {
        float splitDist = cascadeSplits[i];

        glm::vec3 frustumCorners[8] = {
            glm::vec3(-1.0f,  1.0f, 0.0f),
            glm::vec3( 1.0f,  1.0f, 0.0f),
            glm::vec3( 1.0f, -1.0f, 0.0f),
            glm::vec3(-1.0f, -1.0f, 0.0f),
            glm::vec3(-1.0f,  1.0f,  1.0f),
            glm::vec3( 1.0f,  1.0f,  1.0f),
            glm::vec3( 1.0f, -1.0f,  1.0f),
            glm::vec3(-1.0f, -1.0f,  1.0f),
        };

        for (size_t j = 0; j < 8; j++) {
            glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[j], 1.0f);
            frustumCorners[j] = invCorner / invCorner.w;
        }

        for (size_t j = 0; j < 4; j++) {
            glm::vec3 dist = frustumCorners[j + 4] - frustumCorners[j];
            frustumCorners[j + 4] = frustumCorners[j] + (dist * splitDist);
            frustumCorners[j] = frustumCorners[j] + (dist * lastSplitDist);
        }

        // Get frustum center
        glm::vec3 frustumCenter = glm::vec3(0.0f);
        for (size_t j = 0; j < 8; j++) {
            frustumCenter += frustumCorners[j];
        }
        frustumCenter /= 8.0f;

        // Calculate radius based on frustum corners
        float radius = 0.0f;
        for (size_t j = 0; j < 8; j++) {
            float distance = glm::length(frustumCorners[j] - frustumCenter);
            radius = glm::max(radius, distance);
        }

        // Round to avoid flickering, as in your original code
        radius = std::ceil(radius * 16.0f) / 16.0f;

        // This ensures terrain isn't clipped at cascade boundaries
        float extraRadius = 5.0f; // Adjust based on your scene scale

        // For indoor scenes, add more extra coverage to the initial cascades
        if (i == 0) extraRadius *= 2.0f;

        glm::vec3 maxExtents = glm::vec3(radius + extraRadius);
        glm::vec3 minExtents = -maxExtents;

        // Move light source further back to capture more of the scene
        float lightDistanceMultiplier = 1.5f + i * 0.1f; // Increase for further cascades

        glm::vec3 lightPos = frustumCenter - lightDir * (-minExtents.z * lightDistanceMultiplier);
        glm::mat4 lightViewMatrix = glm::lookAt(lightPos, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));

        // CRITICAL FIX: Add extra depth range to avoid clipping shadow casters
        float shadowNearPlane = 0.0f; // Start at frustum center
        float shadowFarPlane = maxExtents.z - minExtents.z + extraRadius * 2.0f; // Add extra depth

        glm::mat4 lightOrthoMatrix = glm::ortho(
            minExtents.x, maxExtents.x,
            minExtents.y, maxExtents.y,
            shadowNearPlane, shadowFarPlane
        );

        // Store split distance and matrix in cascade
        float splitDepth = (nearClip + splitDist * clipRange) * -1.0f;
        lightSpaceArray.matrices[i] = lightOrthoMatrix * lightViewMatrix;
        lightSpaceArray.matrices[i][2][3] = splitDepth;

        lastSplitDist = cascadeSplits[i];
    }
}
