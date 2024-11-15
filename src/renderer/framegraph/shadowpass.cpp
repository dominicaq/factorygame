#include "shadowpass.h"
#include <glad/glad.h>

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
    glClear(GL_DEPTH_BUFFER_BIT);

    m_shadowShader.use();

    // Get atlas data
    std::pair<int, int> atlasDimensions = renderer.getShadowAtlasDimensions();
    int atlasSize = atlasDimensions.first;
    int tileSize = atlasDimensions.second;
    int maxShadowMaps = (atlasSize / tileSize) * (atlasSize / tileSize);

    int tileIndex = 0;
    // Render 2D shadow maps
    registry.view<LightSpaceMatrix, Light>().each([&](auto& lightSpaceMatrix, auto& light) {
        if (tileIndex >= maxShadowMaps) {
            std::cerr << "[Warning] Exceeded maximum number of shadow maps in the atlas!" << std::endl;
            return;
        }

        int tileX = (tileIndex % (atlasSize / tileSize)) * tileSize;
        int tileY = (tileIndex / (atlasSize / tileSize)) * tileSize;
        glViewport(tileX, tileY, tileSize, tileSize);

        light.atlasIndices[0] = tileIndex;
        m_shadowShader.setMat4("u_LightSpaceMatrix", lightSpaceMatrix.matrix);

        renderSceneDepth(renderer, registry);
        ++tileIndex;
    });

    // Render cubemap shadow maps
    registry.view<LightSpaceMatrixCube, Light>().each([&](auto entity, auto& lightSpaceCube, auto& light) {
        for (int face = 0; face < 6; ++face) {
            if (tileIndex >= maxShadowMaps) {
                std::cerr << "[Warning] Exceeded maximum number of shadow maps in the atlas!" << std::endl;
                return;
            }

            int tileX = (tileIndex % (atlasSize / tileSize)) * tileSize;
            int tileY = (tileIndex / (atlasSize / tileSize)) * tileSize;
            glViewport(tileX, tileY, tileSize, tileSize);

            m_shadowShader.setMat4("u_LightSpaceMatrix", lightSpaceCube.matrices[face]);
            light.atlasIndices[face] = tileIndex;

            renderSceneDepth(renderer, registry);
            ++tileIndex;
        }
    });

    shadowAtlas->unbind();
    auto screenDimensions = renderer.getScreenDimensions();
    glViewport(0, 0, screenDimensions.first, screenDimensions.second);
}

void ShadowPass::renderSceneDepth(Renderer& renderer, entt::registry& registry) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    auto viewMesh = registry.view<Mesh*, ModelMatrix>();
    for (auto entity : viewMesh) {
        const auto& mesh = registry.get<Mesh*>(entity);
        const auto& modelMatrix = registry.get<ModelMatrix>(entity);

        m_shadowShader.setMat4("u_Model", modelMatrix.matrix);
        renderer.draw(mesh);
    }
}
