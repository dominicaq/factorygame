#ifndef DEBUGPASS_H
#define DEBUGPASS_H

#include "renderpass.h"
#include "../../globals.h"

class DebugPass : public RenderPass {
public:
    DebugPass(Camera* camera) {
        m_camera = camera;
    };

    void setup() override {
        std::string debugVertexPath = SHADER_DIR + "deferred/debug_gbuff.vs";
        std::string debugFragmentPath = SHADER_DIR + "deferred/debug_gbuff.fs";
        m_debugShader.load(debugVertexPath, debugFragmentPath);
        m_debugShader.use();
        m_debugShader.setFloat("u_Near", m_camera->getNearPlane());
        m_debugShader.setFloat("u_Far",  m_camera->getFarPlane());
    };

    void execute(Renderer& renderer, entt::registry& registry) override {
        if (DEBUG_CTX.mode < 0) {
            return;
        }

        // Bind default framebuffer for debugging
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use the debug shader
        m_debugShader.use();

        Camera* camera = renderer.getCamera();
        if (m_camera != camera) {
            m_camera = camera;
            m_debugShader.setFloat("u_Near", m_camera->getNearPlane());
            m_debugShader.setFloat("u_Far",  m_camera->getFarPlane());
        }

        // Set uniform samplers for G-buffer textures
        m_debugShader.setInt("gPosition", 0);
        m_debugShader.setInt("gNormal", 1);
        m_debugShader.setInt("gAlbedo", 2);
        m_debugShader.setInt("gDepth", 3);

        // Set debug mode (defines what to visualize)
        m_debugShader.setInt("debugMode", DEBUG_CTX.mode);
        m_debugShader.setInt("numSlices", DEBUG_CTX.numDepthSlices);

        // Bind G-buffer textures using the Framebuffer class
        Framebuffer* gBuffer = renderer.getFramebuffer();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gBuffer->getColorAttachment(0)); // Position

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gBuffer->getColorAttachment(1)); // Normal

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gBuffer->getColorAttachment(2)); // Albedo

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, gBuffer->getDepthAttachment()); // Depth

        // Bind additional G-buffer textures if any (NUM_ATTACHMENTS > 4)
        for (int i = 4; i < renderer.getNumAttachments(); ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, gBuffer->getColorAttachment(i));
            m_debugShader.setInt("gExtraTexture" + std::to_string(i - 3), i);
        }

        // Draw the screen-aligned quad to visualize the debug information
        renderer.drawScreenQuad();

        // Unbind the framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    };

private:
    Camera* m_camera;
    Shader m_debugShader;
};

#endif
