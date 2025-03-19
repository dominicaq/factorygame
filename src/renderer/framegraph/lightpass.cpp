#include "lightpass.h"
#include <iostream>
#include <string>
#include <algorithm>

#define LIGHT_BATCH_SIZE 16

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

    // Setup G-buffer textures for lighting (done once for all batches)
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

    // Get the view of lights
    auto view = registry.view<Light, Position>();

    // Count the total number of lights
    int totalLights = 0;
    for (auto entity : view) {
        (void)entity; // Avoid unused variable warning
        totalLights++;
    }

    // Calculate number of batches needed
    int numBatches = (totalLights + LIGHT_BATCH_SIZE - 1) / LIGHT_BATCH_SIZE; // Ceiling division

    // Process lights in batches
    auto lightIter = view.begin();
    for (int batch = 0; batch < numBatches; ++batch) {
        int lightsInBatch = std::min(LIGHT_BATCH_SIZE, totalLights - batch * LIGHT_BATCH_SIZE);

        // Process each light in the current batch
        for (int i = 0; i < lightsInBatch && lightIter != view.end(); ++i, ++lightIter) {
            auto entity = *lightIter;
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
        }

        // Set the actual number of lights in this batch
        m_lightPassShader.setInt("numLights", lightsInBatch);

        // Draw the screen-aligned quad for this batch of lights
        renderer.drawScreenQuad();
    }

    // Disable blending after all lighting passes
    glDisable(GL_BLEND);

    // Re-enable depth testing for subsequent passes
    glEnable(GL_DEPTH_TEST);
}
