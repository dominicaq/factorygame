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
    glGenBuffers(1, &m_lightSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lightSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_LIGHTS * sizeof(LightSSBO), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_lightSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glGenBuffers(1, &m_lightMatrixSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lightMatrixSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_SHADOW_MAPS * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_lightMatrixSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void LightPass::execute(Renderer& renderer, entt::registry& registry) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    m_lightPassShader.use();

    Camera* camera = renderer.getCamera();
    m_lightPassShader.setVec3("u_CameraPosition", camera->getPosition());

    // Collect lights data from the registry
    std::vector<LightSSBO> lightData;
    std::vector<glm::mat4> lightMatrixData;
    std::vector<GLuint> shadowMapHandles;

    int currentMatrixIndex = 0;
    int currentMapIndex = 0;
    registry.view<Light, Position>().each([&](entt::entity entity, const Light& lightComponent, const Position& positionComponent) {
        LightSSBO lightSSBO{};
        lightSSBO.position = positionComponent.position;
        lightSSBO.radius = lightComponent.radius;
        lightSSBO.color = lightComponent.color;
        lightSSBO.intensity = lightComponent.intensity;
        lightSSBO.isPointLight = (lightComponent.type == LightType::Point) ? 1 : 0;
        lightSSBO.castShadow = lightComponent.castsShadows ? 1 : 0;
        lightSSBO.lightMatrixIndex = currentMatrixIndex;
        lightSSBO.shadowMapIndex = currentMapIndex;

        // Retrieve the shadow map handle
        if (lightComponent.depthHandle == 0 || !lightComponent.castsShadows) {
            lightData.push_back(lightSSBO);
            return;
        }

        shadowMapHandles.push_back(lightComponent.depthHandle);
        currentMapIndex += 1;

        // Check if the entity has LightSpaceMatrix (single shadow matrix)
        if (registry.all_of<LightSpaceMatrix>(entity)) {
            const auto& singleMatrix = registry.get<LightSpaceMatrix>(entity);
            lightMatrixData.push_back(singleMatrix.matrix);
            currentMatrixIndex += 1;
        }
        // Check if the entity has LightSpaceMatrixCube (6 matrices for cubemap shadows)
        else if (registry.all_of<LightSpaceMatrixCube>(entity)) {
            const auto& cubeMatrices = registry.get<LightSpaceMatrixCube>(entity);
            for (int i = 0; i < 6; i++) {
                lightMatrixData.push_back(cubeMatrices.matrices[i]);
            }
            currentMatrixIndex += 6;
        }

        lightData.push_back(lightSSBO);
    });

    // Set shader parameters
    int numLights = std::min(static_cast<int>(lightData.size()), MAX_LIGHTS);
    m_lightPassShader.setInt("numLights", numLights);
    if (lightData.size() > MAX_LIGHTS || lightMatrixData.size() > MAX_SHADOW_MAPS) {
        std::cerr << "[Warning] LightPass::execute: Light SSBO overflow! "
        << "Lights: " << lightData.size() << "/" << MAX_LIGHTS
        << ", Shadow maps: " << lightMatrixData.size() << "/" << MAX_SHADOW_MAPS
        << ". Ensure you stay within engine constraints.\n";
    }

    // Bind the SSBO and upload light data
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lightSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, lightData.size() * sizeof(LightSSBO), lightData.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_lightSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lightMatrixSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, lightMatrixData.size() * sizeof(glm::mat4), lightMatrixData.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_lightMatrixSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Set the shadow maps array in the shader
    for (int i = 0; i < shadowMapHandles.size() && i < MAX_SHADOW_MAPS; ++i) {
        glActiveTexture(GL_TEXTURE3 + i); // Start binding from texture slot 3
        glBindTexture(GL_TEXTURE_2D, shadowMapHandles[i]);
        m_lightPassShader.setInt("shadowMaps[" + std::to_string(i) + "]", 3 + i);
    }

    // Set the G-buffer textures in the shader
    Framebuffer* gbuffer = renderer.getFramebuffer();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gbuffer->getColorAttachment(0));
    m_lightPassShader.setInt("gPosition", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gbuffer->getColorAttachment(1));
    m_lightPassShader.setInt("gNormal", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gbuffer->getColorAttachment(2));
    m_lightPassShader.setInt("gAlbedo", 2);

    // Draw the screen quad to apply the lighting pass
    renderer.drawScreenQuad();

    // Restore OpenGL states
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}
