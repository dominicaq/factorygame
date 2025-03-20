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

    // Generate and bind SSBO
    glGenBuffers(1, &m_lightSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lightSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_LIGHTS * sizeof(LightSSBO), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_lightSSBO);
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
    std::vector<GLuint> shadowMapHandles;  // Store shadow maps handles

    registry.view<Light, Position>().each([&](entt::entity entity, const Light& lightComponent, const Position& positionComponent) {
        LightSSBO lightSSBO{};
        lightSSBO.position = positionComponent.position;
        lightSSBO.radius = lightComponent.radius;
        lightSSBO.color = lightComponent.color;
        lightSSBO.intensity = lightComponent.intensity;
        lightSSBO.castShadow = lightComponent.castsShadows;
        lightSSBO.isPointLight = (lightComponent.type == LightType::Point) ? 1 : 0;

        // Retrieve the shadow map handle
        GLuint shadowMapHandle = lightComponent.depthHandle;
        if (shadowMapHandle != 0) {
            shadowMapHandles.push_back(shadowMapHandle);
        }

        // Check if the entity has LightSpaceMatrix (single shadow matrix)
        if (registry.all_of<LightSpaceMatrix>(entity)) {
            const auto& matrixComponent = registry.get<LightSpaceMatrix>(entity);
            lightSSBO.lightSpaceMatrices[0] = matrixComponent.matrix;
        }
        // Check if the entity has LightSpaceMatrixCube (6 matrices for cubemap shadows)
        else if (registry.all_of<LightSpaceMatrixCube>(entity)) {
            const auto& cubeMatrices = registry.get<LightSpaceMatrixCube>(entity);
            for (int i = 0; i < 6; i++) {
                lightSSBO.lightSpaceMatrices[i] = cubeMatrices.matrices[i];
            }
        }

        lightData.push_back(lightSSBO);
    });

    // Set shader parameters
    int numLights = std::min(static_cast<int>(lightData.size()), MAX_LIGHTS);
    m_lightPassShader.setInt("numLights", numLights);

    // Bind the SSBO and upload light data
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lightSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, lightData.size() * sizeof(LightSSBO), lightData.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_lightSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Set the shadow maps array in the shader
    for (int i = 0; i < numLights; ++i) {
        glActiveTexture(GL_TEXTURE3 + i); // Start binding from texture slot 3
        glBindTexture(GL_TEXTURE_2D, shadowMapHandles[i]);
        m_lightPassShader.setInt("shadowMaps[" + std::to_string(i) + "]", 3 + i);  // Set the texture slot for each light's shadow map
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
