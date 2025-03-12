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
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Access and bind the Renderer’s shadow atlas directly
    Framebuffer* shadowAtlas = renderer.getShadowAtlas();
    shadowAtlas->bind();
    glClear(GL_DEPTH_BUFFER_BIT);

    m_shadowShader.use();

    // Get atlas data
    auto atlasDimensions = renderer.getShadowAtlasDimensions();
    int atlasSize = atlasDimensions.first;
    int tileSize = atlasDimensions.second;
    int maxShadowMaps = (atlasSize / tileSize) * (atlasSize / tileSize);

    int tileIndex = 0;

    prepareInstanceMap(registry);

    // Render 2D shadow maps (directional/spotlights) to the atlas
    registry.view<LightSpaceMatrix, Light>().each([&](auto& lightSpaceMatrix, auto& light) {
        if (tileIndex >= maxShadowMaps) {
            std::cerr << "[Warning] Exceeded maximum number of shadow maps in the atlas!" << std::endl;
            return;
        }

        // Compute the viewport for this shadow map
        int tileX = (tileIndex % (atlasSize / tileSize)) * tileSize;
        int tileY = (tileIndex / (atlasSize / tileSize)) * tileSize;
        glViewport(tileX, tileY, tileSize, tileSize);

        light.atlasIndices[0] = tileIndex;
        m_shadowShader.setMat4("u_LightSpaceMatrix", lightSpaceMatrix.matrix);

        // Draw scene from light's point of view
        renderSceneDepth(renderer, registry);
        ++tileIndex;
    });

    // TODO: render cube maps to their own texture, use geometry shader for instancing
    // Render point light shadow maps
    registry.view<LightSpaceMatrixCube, Light>().each([&](auto entity, auto& lightSpaceCube, auto& light) {
        if (tileIndex + 6 > maxShadowMaps) {
            std::cerr << "[Warning] Exceeded maximum number of shadow maps in the atlas!" << std::endl;
            return;
        }

        // Render each face separately
        for (int face = 0; face < 6; ++face) {
            int currentTileIndex = tileIndex + face;

            // Compute the viewport for this face
            int tileX = (currentTileIndex % (atlasSize / tileSize)) * tileSize;
            int tileY = (currentTileIndex / (atlasSize / tileSize)) * tileSize;
            glViewport(tileX, tileY, tileSize, tileSize);

            // Set atlas index for this face
            light.atlasIndices[face] = currentTileIndex;
            m_shadowShader.setMat4("u_LightSpaceMatrix", lightSpaceCube.matrices[face]);

            // Draw the scene from this face's point of view
            renderSceneDepth(renderer, registry);
        }

        // Increment for next group of faces
        tileIndex += 6;
    });

    shadowAtlas->unbind();
    auto screenDimensions = renderer.getScreenDimensions();
    glViewport(0, 0, screenDimensions.first, screenDimensions.second);
}

void ShadowPass::renderSceneDepth(Renderer& renderer, entt::registry& registry) {
    // Normal rendering
    registry.view<Mesh*, ModelMatrix>().each([&](Mesh* mesh, const ModelMatrix& modelMatrix) {
        m_shadowShader.setMat4("u_Model", modelMatrix.matrix);
        renderer.draw(mesh);
    });

    // Instance rendering
    for (const auto& [meshId, matrices] : m_instanceMap) {
        if (matrices.empty()) {
            continue;
        }

        // Set the first matrix as the model uniform (for gl_InstanceID == 0)
        // This ensures the first instance is drawn correctly
        m_shadowShader.setMat4("u_Model", matrices[0]);
        renderer.drawInstanced(meshId);
    }
}

void ShadowPass::prepareInstanceMap(entt::registry& registry) {
    // Clear the previous frame's instance map
    m_instanceMap.clear();

    // Collect all instances by mesh ID
    registry.view<MeshInstance, ModelMatrix>().each([&](MeshInstance& instance, const ModelMatrix& modelMatrix) {
        size_t id = instance.id;
        if (id >= m_meshInstances.size()) {
            return;
        }
        m_instanceMap[id].push_back(modelMatrix.matrix);
    });
}
