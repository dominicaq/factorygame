#ifndef SKYBOXPASS_H
#define SKYBOXPASS_H

#include "renderpass.h"

class SkyboxPass : public RenderPass {
public:
    explicit SkyboxPass() = default;

    void setup() override {
        std::string skyboxVertexPath = ASSET_DIR "shaders/core/skybox.vs";
        std::string skyboxFragmentPath = ASSET_DIR "shaders/core/skybox.fs";
        std::vector<std::string> faces = {
            ASSET_DIR "textures/skyboxes/bspace/1.png", // Left (-X)
            ASSET_DIR "textures/skyboxes/bspace/3.png", // Back (-Z)
            ASSET_DIR "textures/skyboxes/bspace/5.png", // Bottom (-Y)
            ASSET_DIR "textures/skyboxes/bspace/6.png", // Top (+Y)
            ASSET_DIR "textures/skyboxes/bspace/2.png", // Front (+Z)
            ASSET_DIR "textures/skyboxes/bspace/4.png"  // Right (+X)
        };

        if (!m_skyboxShader.load(skyboxVertexPath, skyboxFragmentPath)) {
            std::cerr << "[Error] Renderer::loadSkyboxShader: Failed to create skyboxShader!\n";
        }

        // Load the cubemap textures from image files
        m_skyboxTexture = CubeMap::createFromImages(faces);

        // Get the cubemap vertices from MeshGen
        const float* cubeMapVerts = MeshGen::createCubeMapVerts();

        // Generate the VAO for the skybox
        glGenVertexArrays(1, &m_skyboxVAO);
        glGenBuffers(1, &m_skyboxVBO);

        glBindVertexArray(m_skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, 108 * sizeof(float), cubeMapVerts, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        // Unbind VAO
        glBindVertexArray(0);
    };

    void execute(Renderer& renderer, entt::registry& registry) override {
        // Save ALL relevant OpenGL states
        GLint originalDepthFunc;
        glGetIntegerv(GL_DEPTH_FUNC, &originalDepthFunc);

        GLboolean depthMaskEnabled;
        glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMaskEnabled);

        // Resources
        Camera* camera = renderer.getCamera();
        const glm::mat4& viewMatrix = camera->getViewMatrix();
        glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(viewMatrix));

        // Use skybox shader
        m_skyboxShader.use();
        m_skyboxShader.setMat4("view", viewNoTranslation);
        m_skyboxShader.setMat4("projection", camera->getProjectionMatrix());

        // Explicitly ensure depth testing is enabled
        glEnable(GL_DEPTH_TEST);

        // Disable depth clamp (important for skybox correct rendering)
        glDisable(GL_DEPTH_CLAMP);

        // Change depth function for skybox
        glDepthFunc(GL_LEQUAL);

        // Disable depth writing
        glDepthMask(GL_FALSE);

        // Ensure we're using texture unit 0
        glActiveTexture(GL_TEXTURE0);

        // Bind the skybox texture and VAO
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxTexture);
        glBindVertexArray(m_skyboxVAO);

        // Draw the skybox
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Reset bindings
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        glDepthFunc(originalDepthFunc);
        glDepthMask(depthMaskEnabled);
    }

private:
    unsigned int m_skyboxVAO, m_skyboxVBO;
    unsigned int m_skyboxTexture;
    Shader m_skyboxShader;
};

#endif
