#include "light_system.h"

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
    glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, 1.5f * radius);

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

    // Source:
    // https://johanmedestrom.wordpress.com/2016/03/18/opengl-cascaded-shadow-maps/
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

        float radius = 0.0f;
        for (size_t j = 0; j < 8; j++) {
            float distance = glm::length(frustumCorners[j] - frustumCenter);
            radius = glm::max(radius, distance);
        }
        radius = std::ceil(radius * 16.0f) / 16.0f;

        glm::vec3 maxExtents = glm::vec3(radius);
        glm::vec3 minExtents = -maxExtents;

        glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);

        // Store split distance and matrix in cascade
        float splitDepth = (nearClip + splitDist * clipRange) * -1.0f;
        lightSpaceArray.matrices[i] = lightOrthoMatrix * lightViewMatrix;

        /* Store in matrix for extraction later
        [0][0] [0][1] [0][2] [0][3]  // x-axis transformation
        [1][0] [1][1] [1][2] [1][3]  // y-axis transformation
        [2][0] [2][1] [2][2] [2][3]  // z-axis transformation
        [3][0] [3][1] [3][2] [3][3]  // homogeneous coordinates */
        lightSpaceArray.matrices[i][2][3] = splitDepth;

        lastSplitDist = cascadeSplits[i];
    }
}
