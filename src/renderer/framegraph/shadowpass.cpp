#include "shadowpass.h"
#include <glad/glad.h>

ShadowPass::ShadowPass(Renderer& renderer, unsigned int atlasSize, unsigned int tileSize)
    : m_atlasSize(atlasSize),
      m_tileSize(tileSize) {}

void ShadowPass::setup() {
    std::string shadowVertPath = ASSET_DIR "shaders/core/shadow.vs";
    std::string shadowFragPath = ASSET_DIR "shaders/core/shadow.fs";
    if (!m_shadowShader.load(shadowVertPath, shadowFragPath)) {
        std::cerr << "[Error] Renderer::Renderer: Failed to load shadow shader!\n";
    }
}

void ShadowPass::execute(Renderer& renderer, entt::registry& registry) {
    // Access and bind Renderer’s shadow atlas directly
    Framebuffer* shadowAtlas = renderer.getShadowAtlas();
    shadowAtlas->bind();
    m_shadowShader.use();

    int tileIndex = 0;

    // Render 2D shadow maps
    registry.view<LightSpaceMatrix>().each([&](auto& lightSpaceMatrix) {
        m_shadowShader.setMat4("u_LightSpaceMatrix", lightSpaceMatrix.matrix);

        int tileX = (tileIndex % (m_atlasSize / m_tileSize)) * m_tileSize;
        int tileY = (tileIndex / (m_atlasSize / m_tileSize)) * m_tileSize;

        glViewport(tileX, tileY, m_tileSize, m_tileSize);
        glClear(GL_DEPTH_BUFFER_BIT);

        renderSceneDepth(renderer, registry);
        ++tileIndex;
    });

    // Render cubemap shadow maps
    registry.view<LightSpaceMatrixCube>().each([&](auto& lightSpaceCube) {
        for (int face = 0; face < 6; ++face) {
            m_shadowShader.setMat4("u_LightSpaceMatrix", lightSpaceCube.matrices[face]);

            int tileX = (tileIndex % (m_atlasSize / m_tileSize)) * m_tileSize;
            int tileY = (tileIndex / (m_atlasSize / m_tileSize)) * m_tileSize;

            glViewport(tileX, tileY, m_tileSize, m_tileSize);
            glClear(GL_DEPTH_BUFFER_BIT);

            renderSceneDepth(renderer, registry);
            ++tileIndex;
        }
    });

    shadowAtlas->unbind();
    auto dimensions = renderer.getScreenDimensions();
    glViewport(0, 0, dimensions.first, dimensions.second);
}

void ShadowPass::renderSceneDepth(Renderer& renderer, entt::registry& registry) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    auto viewMesh = registry.view<Mesh*, ModelMatrix>();
    for (auto entity : viewMesh) {
        const auto& mesh = registry.get<Mesh*>(entity);
        const auto& modelMatrix = registry.get<ModelMatrix>(entity);

        m_shadowShader.use();
        m_shadowShader.setMat4("u_Model", modelMatrix.matrix);

        renderer.draw(mesh);
    }
}
