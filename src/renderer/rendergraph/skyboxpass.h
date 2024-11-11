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
            ASSET_DIR "textures/skyboxes/bspace/1.png",
            ASSET_DIR "textures/skyboxes/bspace/3.png",
            ASSET_DIR "textures/skyboxes/bspace/5.png",
            ASSET_DIR "textures/skyboxes/bspace/6.png",
            ASSET_DIR "textures/skyboxes/bspace/2.png",
            ASSET_DIR "textures/skyboxes/bspace/4.png"
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
        // Resources
        Camera* camera = renderer.getCamera();
        const glm::mat4& viewMatrix = camera->getViewMatrix();
        // Remove translation from the view matrix for the skybox
        glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(viewMatrix));

        // Change depth function so the skybox renders behind everything
        glDepthFunc(GL_LEQUAL);

        // Disable depth writing so the skybox doesn't overwrite depth buffer values
        glDepthMask(GL_FALSE);

        // Use skybox shader
        m_skyboxShader.use();

        m_skyboxShader.setMat4("view", viewNoTranslation);
        m_skyboxShader.setMat4("projection", camera->getProjectionMatrix());

        // Bind the skybox VAO and texture
        glBindVertexArray(m_skyboxVAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);  // Draw the skybox

        // Unbind VAO
        glBindVertexArray(0);

        // Re-enable depth writing after drawing the skybox
        glDepthMask(GL_TRUE);

        // Reset depth function to default (GL_LESS)
        glDepthFunc(GL_LESS);
    };

private:
    unsigned int m_skyboxVAO, m_skyboxVBO;
    unsigned int m_skyboxTexture;
    Shader m_skyboxShader;
};

#endif
