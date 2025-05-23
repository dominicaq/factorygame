#include "shadowpass.h"
#include <glad/glad.h>
#include "../../scene/scene.h"

ShadowPass::~ShadowPass() {
    // Clean up the framebuffer
    m_shadowFrameBuffer->~Framebuffer();

    // Clean up all shadow maps
    for (auto& [entity, texture] : m_lightShadowMapMap) {
        glDeleteTextures(1, &texture);
    }
    m_lightShadowMapMap.clear();

    // Clean up all light arrays
    for (auto& [entity, lightArrayTexture] : m_lightArrayMap) {
        glDeleteTextures(1, &lightArrayTexture);
    }
    m_lightArrayMap.clear();
}

void ShadowPass::setup() {
    std::string shadowVertPath = ASSET_DIR "shaders/core/shadow.vs";
    std::string shadowFragPath = ASSET_DIR "shaders/core/shadow.fs";
    if (!m_shadowShader.load(shadowVertPath, shadowFragPath)) {
        std::cerr << "[Error] ShadowPass: Failed to load shadow shader!\n";
    }

    // Create a shared framebuffer for all shadow rendering
    m_shadowFrameBuffer = new Framebuffer(1024, 1024, 0, true);
}

void ShadowPass::execute(Renderer& renderer, entt::registry& registry) {
    // Get shadow config properties
    int shadowRes = renderer.config.shadows.shadowResolution;
    bool enableShadows = renderer.config.shadows.enableShadows;
    if (!enableShadows) {
        return;
    }

    // Save only essential states
    GLint originalFramebuffer;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &originalFramebuffer);

    GLint originalViewport[4];
    glGetIntegerv(GL_VIEWPORT, originalViewport);

    // Setup for shadow rendering - minimal state changes
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    m_shadowShader.use();

    // Render single shadow maps (spotlights)
    registry.view<LightSpaceMatrix, Light>().each([&](auto entity, auto& lightSpaceMatrix, auto& light) {
        if (!light.castShadow || !light.isActive) {
            return;
        }

        // Create or reuse a shadow map for this light
        if (m_lightShadowMapMap.find(entity) == m_lightShadowMapMap.end()) {
            m_lightShadowMapMap[entity] = createShadowMap(shadowRes);
        }

        // Bind framebuffer and attach this light's shadow map
        m_shadowFrameBuffer->bind();
        m_shadowFrameBuffer->resetDepthAttachment();
        unsigned int depthHandle = m_lightShadowMapMap[entity];
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthHandle, 0);
        glClear(GL_DEPTH_BUFFER_BIT);

        // Set the view/projection matrix and render the scene
        glViewport(0, 0, shadowRes, shadowRes);
        m_shadowShader.setMat4("u_LightSpaceMatrix", lightSpaceMatrix.matrix);
        renderSceneDepth(renderer, registry);

        // Store the handle in the light component
        light.depthHandle = depthHandle;
    });

    // Render shadow map arrays
    registry.view<LightSpaceMatrixArray, Light>().each([&](auto entity, auto& lightSpaceArray, auto& light) {
        if (!light.castShadow || !light.isActive) {
            return;
        }

        int numFaces = 6;
        if (light.type == LightType::Directional) {
            numFaces = renderer.config.shadows.cascades.numCascades;
            shadowRes = renderer.config.shadows.directionalLightResolution;
        }

        // Create or reuse a cubemap for this light
        if (m_lightArrayMap.find(entity) == m_lightArrayMap.end()) {
            m_lightArrayMap[entity] = createLightStrip(shadowRes, numFaces);
        }

        // Get the cubemap for this light
        unsigned int depthAtlas = m_lightArrayMap[entity];

        // Bind the framebuffer using the custom framebuffer
        m_shadowFrameBuffer->bind();
        m_shadowFrameBuffer->resetDepthAttachment();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthAtlas, 0);
        glClear(GL_DEPTH_BUFFER_BIT);

        // Render each face to its section in the atlas
        for (int face = 0; face < numFaces; ++face) {
            // Calculate the viewport position for this face
            int xOffset = face * shadowRes;
            glViewport(xOffset, 0, shadowRes, shadowRes);

            // Extract split depth from matrix (we placed this var into the matrix)
            float splitDepth = 0.0f;
            if (light.type == LightType::Directional) {
                splitDepth = lightSpaceArray.matrices[face][2][3];
                lightSpaceArray.matrices[face][2][3] = 0.0f;
            }

            // Render the scene from the face index
            m_shadowShader.setMat4("u_LightSpaceMatrix", lightSpaceArray.matrices[face]);
            renderSceneDepth(renderer, registry);

            // Propigate split depth to the shader
            lightSpaceArray.matrices[face][2][3] = splitDepth;
        }

        // Store the atlas handle in the light component
        shadowRes = renderer.config.shadows.shadowResolution;
        light.depthHandle = depthAtlas;
    });

    // Reset only what's necessary
    m_shadowFrameBuffer->bind();
    m_shadowFrameBuffer->resetDepthAttachment();
    m_shadowFrameBuffer->unbind();

    // Restore original framebuffer and viewport
    glBindFramebuffer(GL_FRAMEBUFFER, originalFramebuffer);
    glViewport(originalViewport[0], originalViewport[1], originalViewport[2], originalViewport[3]);

    // Critical - disable depth clamp which was affecting skybox
    glDisable(GL_DEPTH_CLAMP);

    // Reset texture bindings
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ShadowPass::renderSceneDepth(Renderer& renderer, entt::registry& registry) {
    renderer.clearDrawCommands();
    m_shadowBatch.clear();

    // Batch draw
    registry.view<Mesh, ModelMatrix>().each([&](const Mesh& mesh, const ModelMatrix& modelMatrix) {
        if (mesh.material->isDeferred) {
            m_shadowBatch.addInstance(mesh, modelMatrix.matrix, mesh.material->uvScale);
        }
    });

    // Draw scene
    m_shadowBatch.prepare(renderer);
    m_shadowBatch.render(renderer);
}

void ShadowPass::cleanupLightResources(entt::entity lightEntity) {
    // Clean up directional/spot light shadow map
    auto shadowIt = m_lightShadowMapMap.find(lightEntity);
    if (shadowIt != m_lightShadowMapMap.end()) {
        glDeleteTextures(1, &shadowIt->second);
        m_lightShadowMapMap.erase(shadowIt);
    }

    // Clean up point light cubemap
    auto cubemapIt = m_lightArrayMap.find(lightEntity);
    if (cubemapIt != m_lightArrayMap.end()) {
        glDeleteTextures(1, &cubemapIt->second);
        m_lightArrayMap.erase(cubemapIt);
    }
}

unsigned int ShadowPass::createShadowMap(int shadowRes) {
    unsigned int shadowMap;
    glGenTextures(1, &shadowMap);
    glBindTexture(GL_TEXTURE_2D, shadowMap);

    // Consider using GL_DEPTH_COMPONENT24 for better precision/performance balance
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24,
                shadowRes, shadowRes,
                0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    return shadowMap;
}

unsigned int ShadowPass::createLightStrip(int shadowRes, int numFaces) {
    unsigned int cubemap;
    glGenTextures(1, &cubemap);
    glBindTexture(GL_TEXTURE_2D, cubemap);

    // Create a 2D texture with the width being numFaces * SHADOW_RESOLUTION and the height being SHADOW_RESOLUTION
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, numFaces * shadowRes, shadowRes, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);  // Initialize with NULL, which sets all depth values to 1.0

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return cubemap;
}
