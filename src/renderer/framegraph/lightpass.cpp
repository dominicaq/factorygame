#include "lightpass.h"
#include <iostream>
#include <string>
#include <algorithm>

// TODO: clustered deferred lighting
// https://www.aortiz.me/2018/12/21/CG.html
void LightPass::setup() {
    std::string lightVertexPath = ASSET_DIR "shaders/core/deferred/lightpass.vs";
    std::string lightFragmentPath = ASSET_DIR "shaders/core/deferred/lightpass.fs";
    if (!m_lightPassShader.load(lightVertexPath, lightFragmentPath)) {
        std::cerr << "[Error] LightPass::setup: Failed to create lightPassShader!\n";
    }
}

void LightPass::execute(Renderer& renderer, entt::registry& registry) {
    // Bind default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Screen space light calculation, don't need depth
    glDisable(GL_DEPTH_TEST);

    // Enable additive blending for lighting accumulation
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    // Use Light Pass Shader
    m_lightPassShader.use();

    // Set the camera position in the shader
    Camera* camera = renderer.getCamera();
    m_lightPassShader.setVec3("u_CameraPosition", camera->getPosition());

    // Get the maximum number of lights the shader can handle
    // const int MAX_LIGHTSs = 20; // Should match the MAX_LIGHTS define in the shader

    // Limit the number of lights we process to MAX_LIGHTS
    auto view = registry.view<Light, Position>();
    // int lightCount = std::min(static_cast<int>(view.size()), MAX_LIGHTSs);

    // Pass each light's data to the shader
    int i = 0;
    for (auto entity : view) {
        if (i >= MAX_LIGHTS) break;

        const auto& [lightComponent, positionComponent] = view.get<Light, Position>(entity);
        std::string indexStr = "[" + std::to_string(i) + "]";

        // Light properties (always set these)
        m_lightPassShader.setVec3("lights" + indexStr + ".position", positionComponent.position);
        m_lightPassShader.setVec3("lights" + indexStr + ".color", lightComponent.color);
        m_lightPassShader.setFloat("lights" + indexStr + ".intensity", lightComponent.intensity);
        m_lightPassShader.setFloat("lights" + indexStr + ".radius", lightComponent.radius);
        m_lightPassShader.setBool("lights" + indexStr + ".castsShadows", lightComponent.castsShadows);

        // Only set shadow-related data if the light casts shadows
        if (lightComponent.castsShadows) {
            // Atlas indices
            for (size_t j = 0; j < 6; ++j) {
                std::string atlasIndexStr = "lights" + indexStr + ".atlasIndices[" + std::to_string(j) + "]";
                m_lightPassShader.setInt(atlasIndexStr, lightComponent.atlasIndices[j]);
            }

            // Light-space matrix
            if (auto* lightSpaceMatrix = registry.try_get<LightSpaceMatrix>(entity)) {
                m_lightPassShader.setMat4("lights" + indexStr + ".lightSpaceMatrices[0]", lightSpaceMatrix->matrix);
            } else if (auto* lightSpaceMatrices = registry.try_get<LightSpaceMatrixCube>(entity)) {
                // Cubemap light-space matrices (6 faces)
                for (size_t j = 0; j < 6; ++j) {
                    std::string matrixStr = "lights" + indexStr + ".lightSpaceMatrices[" + std::to_string(j) + "]";
                    m_lightPassShader.setMat4(matrixStr, lightSpaceMatrices->matrices[j]);
                }
            }
        }

        ++i;
    }

    // Set the actual number of lights
    m_lightPassShader.setInt("numLights", i);

    // Setup G-buffer textures for lighting
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

    // Set shadow atlas for sampling
    Framebuffer* shadowAtlas = renderer.getShadowAtlas();
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, shadowAtlas->getDepthAttachment());
    m_lightPassShader.setInt("u_ShadowAtlas", 3);

    // Shadow Atlas properties
    std::pair<int, int> atlasDimensions = renderer.getShadowAtlasDimensions();
    m_lightPassShader.setInt("u_AtlasSize", atlasDimensions.first);
    m_lightPassShader.setInt("u_TileSize", atlasDimensions.second);

    // Draw the screen-aligned quad for lighting
    renderer.drawScreenQuad();

    // Disable blending after the lighting pass
    glDisable(GL_BLEND);

    // Re-enable depth testing for subsequent passes
    glEnable(GL_DEPTH_TEST);
}
