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

    // Reserve the shadow vectors
    m_lightMatrixData.reserve(MAX_SHADOW_MAPS * 6);
    m_shadowMapHandles.reserve(MAX_SHADOW_MAPS);

    // Load the skybox
    std::string skyboxVertexPath = ASSET_DIR "shaders/core/skybox.vs";
    std::string skyboxFragmentPath = ASSET_DIR "shaders/core/skybox.fs";
    std::vector<std::string> faces = {
        ASSET_DIR "textures/skyboxes/bspace/1.png",
        ASSET_DIR "textures/skyboxes/bspace/3.png",
        ASSET_DIR "textures/skyboxes/bspace/5.png",
        ASSET_DIR "textures/skyboxes/bspace/6.png",
        ASSET_DIR "textures/skyboxes/bspace/2.png",
        ASSET_DIR "textures/skyboxes/bspace/4.png"
    };

    // Load the cubemap textures from image files
    m_skyboxTexture = CubeMap::createFromImages(faces);
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
            m_lightData.push_back(lightSSBO);
            return;
        }

        m_shadowMapHandles.push_back(lightComponent.depthHandle);
        currentMapIndex += 1;

        // Check if the entity has LightSpaceMatrix (single shadow matrix)
        if (registry.all_of<LightSpaceMatrix>(entity)) {
            const auto& singleMatrix = registry.get<LightSpaceMatrix>(entity);
            m_lightMatrixData.push_back(singleMatrix.matrix);
            currentMatrixIndex += 1;
        }
        // Check if the entity has LightSpaceMatrixCube (6 matrices for cubemap shadows)
        else if (registry.all_of<LightSpaceMatrixCube>(entity)) {
            const auto& cubeMatrices = registry.get<LightSpaceMatrixCube>(entity);
            for (int i = 0; i < 6; i++) {
                m_lightMatrixData.push_back(cubeMatrices.matrices[i]);
            }
            currentMatrixIndex += 6;
        }

        m_lightData.push_back(lightSSBO);
    });

    // Set shader parameters
    int numLights = std::min(static_cast<int>(m_lightData.size()), MAX_LIGHTS);
    m_lightPassShader.setInt("numLights", numLights);
    if (m_lightData.size() > MAX_LIGHTS || m_lightMatrixData.size() > MAX_SHADOW_MAPS * 6) {
        std::cerr << "[Warning] LightPass::execute: Light SSBO overflow! "
        << "Lights: " << m_lightData.size() << "/" << MAX_LIGHTS
        << ", Shadow maps: " << m_lightMatrixData.size() << "/" << MAX_SHADOW_MAPS * 6
        << ". Ensure you stay within engine constraints.\n";
    }

    // Bind the SSBO and upload light data
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lightSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_lightData.size() * sizeof(LightSSBO), m_lightData.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_lightSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lightMatrixSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_lightMatrixData.size() * sizeof(glm::mat4), m_lightMatrixData.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_lightMatrixSSBO);
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

    // Set the shadow maps array in the shader
    for (int i = 0; i < m_shadowMapHandles.size() && i < MAX_SHADOW_MAPS; ++i) {
        glActiveTexture(GL_TEXTURE4 + i); // Start binding from texture slot 4
        glBindTexture(GL_TEXTURE_2D, m_shadowMapHandles[i]);
        m_lightPassShader.setInt("shadowMaps[" + std::to_string(i) + "]", 4 + i);
    }

    int skyboxTextureUnit = 4 + MAX_SHADOW_MAPS;

    // Set the skybox texture
    glActiveTexture(GL_TEXTURE0 + skyboxTextureUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxTexture);
    m_lightPassShader.setInt("skybox", skyboxTextureUnit);

    // Draw the screen quad to apply the lighting pass
    renderer.drawScreenQuad();

    // Clear the vectors but keep reserved memory for next frame
    m_lightMatrixData.resize(0);
    m_shadowMapHandles.resize(0);
    m_lightData.resize(0);

    // Restore OpenGL states
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}
