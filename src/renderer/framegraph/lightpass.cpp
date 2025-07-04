#include "lightpass.h"
#include <iostream>
#include <string>
#include <algorithm>

void LightPass::setup() {
    std::string lightVertexPath = ASSET_DIR "shaders/core/deferred/lightpass.vs";
    std::string lightFragmentPath = ASSET_DIR "shaders/core/deferred/lightpass.fs";
    if (!m_lightPassShader.load(lightVertexPath, lightFragmentPath)) {
        std::cerr << "[Error] LightPass::setup: Failed to create lightPassShader!\n";
    }

    // Generate and bind SSBOs
    glGenBuffers(1, &m_pointSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_pointSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_LIGHTS * sizeof(PointLightSSBO), nullptr, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_pointSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glGenBuffers(1, &m_spotSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_spotSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_LIGHTS * sizeof(SpotLightSSBO), nullptr, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_spotSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glGenBuffers(1, &m_lightMatrixSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lightMatrixSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_SHADOW_MAPS * sizeof(glm::mat4), nullptr, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_lightMatrixSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Reserve the shadow vectors
    m_lightMatrixData.reserve(MAX_SHADOW_MAPS * 6);
    m_shadowMapHandles.reserve(MAX_SHADOW_MAPS);
}

void LightPass::execute(entt::registry& registry, Camera& camera, Renderer& renderer) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Enable depth test to only affect geometry pixels
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); // Only light where geometry exists

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    m_lightPassShader.use();

    m_lightPassShader.setVec3("u_CameraPosition", camera.getPosition());
    m_lightPassShader.setMat4("view", camera.getViewMatrix());

    int pointCount = 0, spotCount = 0, directionalCount = 0;
    int currentMatrixIndex = 0;
    int currentMapIndex = 0;
    const auto& view = registry.view<Light, Position, Rotation>();
    for (const auto& entity : view) {
        const Light& lightComponent = view.get<Light>(entity);
        const Position& positionComponent = view.get<Position>(entity);
        const Rotation& rotationComponent = view.get<Rotation>(entity);

        if (!lightComponent.isActive) {
            continue;
        }

        switch (lightComponent.type) {
            case LightType::Point: {
                pointCount += 1;

                PointLightSSBO pointSSBO{};
                pointSSBO.position = positionComponent.position;
                pointSSBO.radius = lightComponent.point.radius;
                pointSSBO.color = lightComponent.color;
                pointSSBO.intensity = lightComponent.intensity;
                pointSSBO.castShadow = lightComponent.castShadow ? 1 : 0;
                pointSSBO.lightMatrixIndex = currentMatrixIndex;
                pointSSBO.shadowMapIndex = currentMapIndex;

                // Retrieve the shadow map handle
                if (lightComponent.depthHandle == 0 || !lightComponent.castShadow) {
                    m_pointData.push_back(pointSSBO);
                    continue;
                }

                // Get the shadow map this light created
                m_shadowMapHandles.push_back(lightComponent.depthHandle);
                currentMapIndex += 1;

                // Get matrices associated with this light (6 faces)
                if (registry.all_of<LightSpaceMatrixArray>(entity)) {
                    const auto& cubeMatrixComponent = registry.get<LightSpaceMatrixArray>(entity);

                    for (int i = 0; i < 6; i++) {
                        m_lightMatrixData.push_back(cubeMatrixComponent.matrices[i]);
                    }
                    currentMatrixIndex += 6;
                }
                m_pointData.push_back(pointSSBO);
                break;
            }
            case LightType::Spot: {
                spotCount += 1;

                SpotLightSSBO spotSSBO{};
                spotSSBO.position = positionComponent.position;
                glm::vec3 direction = rotationComponent.quaternion * glm::vec3(0.0f, 0.0f, -1.0f);
                spotSSBO.direction = glm::normalize(direction);
                spotSSBO.color = lightComponent.color;
                spotSSBO.intensity = lightComponent.intensity;

                spotSSBO.innerCutoff = lightComponent.spot.innerCutoff;
                spotSSBO.outerCutoff = lightComponent.spot.outerCutoff;
                spotSSBO.range = lightComponent.spot.range;

                // Shadow data
                spotSSBO.castShadow = lightComponent.castShadow ? 1 : 0;
                spotSSBO.shadowMapIndex = currentMapIndex;
                spotSSBO.lightMatrixIndex = currentMatrixIndex;

                // No shadow casting, dont need to do anything else
                if (lightComponent.depthHandle == 0 || !lightComponent.castShadow) {
                    m_spotData.push_back(spotSSBO);
                    continue;
                }

                // Get the shadow map this light created
                m_shadowMapHandles.push_back(lightComponent.depthHandle);
                currentMapIndex += 1;

                // Get matrix associated with this light
                if (registry.all_of<LightSpaceMatrix>(entity)) {
                    const auto& lightComponent = registry.get<LightSpaceMatrix>(entity);
                    m_lightMatrixData.push_back(lightComponent.matrix);
                    currentMatrixIndex += 1;
                }
                m_spotData.push_back(spotSSBO);
                break;
            }
            case LightType::Directional: {
                std::string base = "directionalLights[" + std::to_string(directionalCount) + "].";
                directionalCount += 1;

                // Set light properties
                glm::vec3 direction = rotationComponent.quaternion * glm::vec3(0.0f, 0.0f, -1.0f);
                m_lightPassShader.setVec3(base + "dir", glm::normalize(direction));

                // Set color and intensity
                m_lightPassShader.setVec3(base + "color", lightComponent.color);
                m_lightPassShader.setFloat(base + "intensity", lightComponent.intensity);

                // Set shadow properties
                m_lightPassShader.setInt(base + "castShadow", lightComponent.castShadow ? 1 : 0);

                // No shadow casting, dont need to do anything else
                if (lightComponent.depthHandle == 0 || !lightComponent.castShadow) {
                    continue;
                }

                // Get the shadow map this light created
                m_shadowMapHandles.push_back(lightComponent.depthHandle);
                m_lightPassShader.setInt(base + "shadowMapIndex", currentMapIndex);
                currentMapIndex += 1;

                // Get matrices associated with this light for cascading shadows
                if (registry.all_of<LightSpaceMatrixArray>(entity)) {
                    auto& lightSpaceArray = registry.get<LightSpaceMatrixArray>(entity);

                    int numCascades = renderer.config.shadows.cascades.numCascades;
                    m_lightPassShader.setInt(base + "numCascades", numCascades);
                    m_lightPassShader.setInt(base + "lightMatrixIndex", currentMatrixIndex);

                    for (int i = 0; i < numCascades; i++) {
                        // Send cascade data
                        float sliceDepth = lightSpaceArray.matrices[i][2][3];
                        m_lightPassShader.setFloat(base + "cascadeSliceDepths[" + std::to_string(i) + "]", sliceDepth);

                        // Send light space data
                        lightSpaceArray.matrices[i][2][3] = 0.0f;
                        m_lightMatrixData.push_back(lightSpaceArray.matrices[i]);
                    }
                    currentMatrixIndex += numCascades;
                }

                break;
            }
        }
    }

    // Safety check
    int numLights = std::min(pointCount + spotCount, MAX_LIGHTS);
    if (numLights > MAX_LIGHTS || m_lightMatrixData.size() > MAX_SHADOW_MAPS * 6) {
        std::cerr << "[Warning] LightPass::execute: Light SSBO overflow! "
        << "Lights: " << numLights << "/" << MAX_LIGHTS
        << ", Shadow maps: " << m_lightMatrixData.size() << "/" << MAX_SHADOW_MAPS * 6
        << ". Ensure you stay within engine constraints.\n";
    }

    m_lightPassShader.setInt("numPointLights", pointCount);
    m_lightPassShader.setInt("numSpotLights", spotCount);
    m_lightPassShader.setInt("numDirectionalLights", directionalCount);

    // Point light SSBO
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_pointSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_pointData.size() * sizeof(PointLightSSBO), m_pointData.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_pointSSBO);

    // Spot light SSBO
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_spotSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_spotData.size() * sizeof(SpotLightSSBO), m_spotData.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_spotSSBO);

    // Matrix data for all lights
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lightMatrixSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_lightMatrixData.size() * sizeof(glm::mat4), m_lightMatrixData.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_lightMatrixSSBO);

    // Unbind
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Set the G-buffer textures in the shader
    Framebuffer* gBuffer = renderer.getFramebuffer();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gBuffer->getColorAttachment(0));
    m_lightPassShader.setInt("gPosition", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gBuffer->getColorAttachment(1));
    m_lightPassShader.setInt("gNormal", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gBuffer->getColorAttachment(2));
    m_lightPassShader.setInt("gAlbedo", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gBuffer->getColorAttachment(3));
    m_lightPassShader.setInt("gPBRParams", 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, gBuffer->getColorAttachment(4));
    m_lightPassShader.setInt("gEmissive", 4);

    // Set the shadow maps array in the shader
    for (int i = 0; i < m_shadowMapHandles.size() && i < MAX_SHADOW_MAPS; ++i) {
        glActiveTexture(GL_TEXTURE5 + i); // Start binding from texture slot 4
        glBindTexture(GL_TEXTURE_2D, m_shadowMapHandles[i]);
        m_lightPassShader.setInt("shadowMaps[" + std::to_string(i) + "]", 5 + i);
    }

    int skyboxTextureUnit = 5 + MAX_SHADOW_MAPS;

    // Set the skybox texture
    glActiveTexture(GL_TEXTURE0 + skyboxTextureUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxTexture);
    m_lightPassShader.setInt("skybox", skyboxTextureUnit);

    // Draw the screen quad to apply the lighting pass
    renderer.drawScreenQuad();

    // Clear the vectors but keep reserved memory for next frame
    m_lightMatrixData.resize(0);
    m_shadowMapHandles.resize(0);
    m_pointData.resize(0);
    m_spotData.resize(0);

    // Restore OpenGL states
    glDisable(GL_BLEND);
    glDepthFunc(GL_LESS);
}
