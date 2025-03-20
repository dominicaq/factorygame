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

    // Clean up all cubemaps
    for (auto& [entity, cubemap] : m_lightCubemapMap) {
        glDeleteTextures(1, &cubemap);
    }
    m_lightCubemapMap.clear();
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
    int shadowRes = renderer.config.shadowResolution;
    bool enableShadows = renderer.config.enableShadows;
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

    // Render 2D shadow maps (directional/spotlights)
    registry.view<LightSpaceMatrix, Light>().each([&](auto entity, auto& lightSpaceMatrix, auto& light) {
        if (!light.castsShadows) {
            return;
        }

        // Create or reuse a shadow map for this light
        if (m_lightShadowMapMap.find(entity) == m_lightShadowMapMap.end()) {
            m_lightShadowMapMap[entity] = createShadowMap(shadowRes);
        }

        // Bind framebuffer and attach this light's shadow map
        m_shadowFrameBuffer->bind();
        m_shadowFrameBuffer->resetDepthAttachment();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_TEXTURE_2D, m_lightShadowMapMap[entity], 0);

        glClear(GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, shadowRes, shadowRes);

        // Set the view/projection matrix and render the scene
        m_shadowShader.setMat4("u_LightSpaceMatrix", lightSpaceMatrix.matrix);
        renderSceneDepth(renderer, registry);

        // Store the handle in the light component
        light.depthHandle = m_lightShadowMapMap[entity];
    });

    // Render cubemaps for point lights
    registry.view<LightSpaceMatrixCube, Light>().each([&](auto entity, auto& lightSpaceCube, auto& light) {
        if (!light.castsShadows) {
            return;
        }

        // Create or reuse a cubemap for this light
        if (m_lightCubemapMap.find(entity) == m_lightCubemapMap.end()) {
            m_lightCubemapMap[entity] = createCubeMapAtlas(shadowRes);
        }

        // Get the cubemap for this light
        unsigned int depthAtlas = m_lightCubemapMap[entity];

        // Bind the framebuffer using the custom framebuffer
        m_shadowFrameBuffer->bind();
        m_shadowFrameBuffer->resetDepthAttachment();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthAtlas, 0);
        glClear(GL_DEPTH_BUFFER_BIT);

        // Render each face to its section in the atlas
        for (int face = 0; face < 6; ++face) {
            // Calculate the viewport position for this face
            int xOffset = face * shadowRes;
            glViewport(xOffset, 0, shadowRes, shadowRes);

            // Render the scene from the face index
            m_shadowShader.setMat4("u_LightSpaceMatrix", lightSpaceCube.matrices[face]);
            renderSceneDepth(renderer, registry);
        }

        // Store the atlas handle in the light component
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
    // Normal rendering of non-instanced meshes
    registry.view<Mesh*, ModelMatrix>().each([&](Mesh* mesh, const ModelMatrix& modelMatrix) {
        if (mesh->wireframe) {
            return;
        }
        m_shadowShader.setMat4("u_Model", modelMatrix.matrix);
        renderer.draw(mesh);
    });

    // Get scene data for instanced rendering
    const auto& instanceMap = m_scene->getInstanceMap();
    const auto& meshInstances = m_scene->getMeshInstances();

    // Render all instances using the shared instance map
    for (const auto& [meshId, matrices] : instanceMap) {
        if (matrices.empty() || meshId >= meshInstances.size() || meshInstances[meshId]->wireframe) {
            continue;
        }

        // Set the first matrix as the model uniform (for gl_InstanceID == 0)
        m_shadowShader.setMat4("u_Model", matrices[0]);
        renderer.drawInstanced(meshId);
    }
}

void ShadowPass::cleanupLightResources(entt::entity lightEntity) {
    // Clean up directional/spot light shadow map
    auto shadowIt = m_lightShadowMapMap.find(lightEntity);
    if (shadowIt != m_lightShadowMapMap.end()) {
        glDeleteTextures(1, &shadowIt->second);
        m_lightShadowMapMap.erase(shadowIt);
    }

    // Clean up point light cubemap
    auto cubemapIt = m_lightCubemapMap.find(lightEntity);
    if (cubemapIt != m_lightCubemapMap.end()) {
        glDeleteTextures(1, &cubemapIt->second);
        m_lightCubemapMap.erase(cubemapIt);
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

unsigned int ShadowPass::createCubeMapAtlas(int shadowRes) {
    unsigned int cubemap;
    glGenTextures(1, &cubemap);
    glBindTexture(GL_TEXTURE_2D, cubemap);

    // Create a 2D texture with the width being 6 * SHADOW_RESOLUTION and the height being SHADOW_RESOLUTION
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, shadowRes * 6, shadowRes, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);  // Initialize with NULL, which sets all depth values to 1.0

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return cubemap;
}
