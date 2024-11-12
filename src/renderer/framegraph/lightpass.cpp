#include "lightpass.h"
#include <iostream>
#include <string>

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

    // Set the number of lights
    auto lightView = registry.view<Light>();
    size_t numLights = lightView.size();

    m_lightPassShader.setInt("numLights", numLights);

    // Pass each light's data to the shader
    size_t i = 0;
    for (auto entity : lightView) {
        std::string indexStr = "[" + std::to_string(i) + "]";

        const auto& lightComponent = registry.get<Light>(entity);
        const auto& positionComponent = registry.get<Position>(entity);

        m_lightPassShader.setVec3("lights" + indexStr + ".position", positionComponent.position);
        m_lightPassShader.setVec3("lights" + indexStr + ".color", lightComponent.color);
        m_lightPassShader.setFloat("lights" + indexStr + ".intensity", lightComponent.intensity);

        ++i;
    }

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

    // Draw the screen-aligned quad for lighting
    renderer.drawScreenQuad();

    // Disable blending after the lighting pass
    glDisable(GL_BLEND);

    // Re-enable depth testing for subsequent passes
    glEnable(GL_DEPTH_TEST);
}
