#include "renderer.h"
#include "framebuffer.h"
#include <iostream>
#include <memory>

// Define the number of G-buffer attachments
#define NUM_ATTACHMENTS 4
#define MAX_LIGHTS 100

void Renderer::setupRenderGraph() {
    // Init shaders
    std::string gBufferVertexPath = ASSET_DIR "shaders/core/deferred/gbuff.vs";
    std::string gBufferFragmentPath = ASSET_DIR "shaders/core/deferred/gbuff.fs";
    if (!m_gBufferShader.load(gBufferVertexPath, gBufferFragmentPath)) {
        std::cerr << "[Error] Renderer::Renderer: Failed to create gBufferShader!\n";
    }

    std::string lightVertexPath = ASSET_DIR "shaders/core/deferred/lightpass.vs";
    std::string lightFragmentPath = ASSET_DIR "shaders/core/deferred/lightpass.fs";
    if (!m_lightPassShader.load(lightVertexPath, lightFragmentPath)) {
        std::cerr << "[Error] Renderer::Renderer: Failed to create lightPassShader!\n";
    }
}

Renderer::Renderer(int width, int height, Camera* camera) {
    m_camera = camera;
    m_width = width;
    m_height = height;

    initOpenGLState();
    initScreenQuad();

    // Initialize G-buffer
    m_gBuffer = std::make_unique<Framebuffer>(width, height, NUM_ATTACHMENTS, true);

    // Set up the skybox shader
    std::string skyboxVertexPath = ASSET_DIR "shaders/core/skybox.vs";
    std::string skyboxFragmentPath = ASSET_DIR "shaders/core/skybox.fs";
    if (!m_skyboxShader.load(skyboxVertexPath, skyboxFragmentPath)) {
        std::cerr << "[Error] Renderer::loadSkyboxShader: Failed to create skyboxShader!\n";
    }

    // Initialize skybox with cubemap faces
    std::vector<std::string> faces = {
        ASSET_DIR "textures/skyboxes/bspace/1.png",
        ASSET_DIR "textures/skyboxes/bspace/3.png",
        ASSET_DIR "textures/skyboxes/bspace/5.png",
        ASSET_DIR "textures/skyboxes/bspace/6.png",
        ASSET_DIR "textures/skyboxes/bspace/2.png",
        ASSET_DIR "textures/skyboxes/bspace/4.png"
    };
    initSkybox(faces);

    // Set up render passes directly in the RenderGraph
    setupRenderGraph();

    // Generate and bind the light buffer
    glGenBuffers(1, &m_lightSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lightSSBO);

    // Allocate buffer with maximum expected size
    size_t bufferSize = sizeof(PointLight) * MAX_LIGHTS;
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_DYNAMIC_DRAW);

    // Bind to binding point 0
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_lightSSBO);

    // Unbind the buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

Renderer::~Renderer() {
    // Clean up mesh buffers
    for (auto& data : m_meshData) {
        if (data.VAO) {
            glDeleteVertexArrays(1, &data.VAO);
        }
        if (data.VBO) {
            glDeleteBuffers(1, &data.VBO);
        }
        if (data.EBO) {
            glDeleteBuffers(1, &data.EBO);
        }
    }

    // Clean up quad
    if (m_quadVAO) {
        glDeleteVertexArrays(1, &m_quadVAO);
    }
}

/*
 * OpenGL State Initialization
 */
void Renderer::initOpenGLState() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Face Culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Disable Blending by default
    glDisable(GL_BLEND);
}

/*
 * Mesh management
 */
void Renderer::draw(const Mesh* mesh) {
    size_t index = mesh->id;
    if (index >= m_meshData.size() || m_meshData[index].VAO == 0) {
        std::cerr << "[Error] Remderer::draw: Mesh buffer id not found!\n";
        return;
    }

    const MeshData& data = m_meshData[index];

    // Bind VAO
    glBindVertexArray(data.VAO);

    // Draw the mesh
    if (data.EBO != 0) {
        glDrawElements(GL_TRIANGLES, data.indexCount, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, data.vertexCount);
    }

    // Unbind VAO
    glBindVertexArray(0);
}

void Renderer::initMeshBuffers(Mesh* mesh, bool isStatic) {
    if (mesh->uvs.empty() ||
        mesh->normals.empty() ||
        mesh->tangents.empty() ||
        mesh->bitangents.empty()) {
        std::cerr << "[Error] Renderer::initMeshBuffers: UVs, normals, tangents, and bitangents must be provided for all vertices.\n";
        return;
    }

    // Assign an ID if not already assigned
    if (mesh->id == SIZE_MAX) {
        mesh->id = m_meshData.size();
    }

    MeshData data = {};
    glGenVertexArrays(1, &data.VAO);
    glBindVertexArray(data.VAO);

    // Build mesh buffer data (14 floats per vertex)
    std::vector<float> bufferData;
    size_t numVertices = mesh->vertices.size();
    bufferData.reserve(numVertices * 14);

    for (size_t i = 0; i < numVertices; ++i) {
        // Positions
        bufferData.push_back(mesh->vertices[i].x);
        bufferData.push_back(mesh->vertices[i].y);
        bufferData.push_back(mesh->vertices[i].z);
        // UVs
        bufferData.push_back(mesh->uvs[i].x);
        bufferData.push_back(mesh->uvs[i].y);
        // Normals
        bufferData.push_back(mesh->normals[i].x);
        bufferData.push_back(mesh->normals[i].y);
        bufferData.push_back(mesh->normals[i].z);
        // Tangents
        bufferData.push_back(mesh->tangents[i].x);
        bufferData.push_back(mesh->tangents[i].y);
        bufferData.push_back(mesh->tangents[i].z);
        // Bitangents
        bufferData.push_back(mesh->bitangents[i].x);
        bufferData.push_back(mesh->bitangents[i].y);
        bufferData.push_back(mesh->bitangents[i].z);
    }

    glGenBuffers(1, &data.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, data.VBO);
    glBufferData(GL_ARRAY_BUFFER, bufferData.size() * sizeof(float), bufferData.data(), GL_STATIC_DRAW);

    if (!mesh->indices.empty()) {
        glGenBuffers(1, &data.EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size() * sizeof(unsigned int), mesh->indices.data(), GL_STATIC_DRAW);
        data.indexCount = static_cast<GLsizei>(mesh->indices.size());
        data.vertexCount = 0;
    } else {
        // No indices
        data.EBO = 0;
        data.indexCount = 0;
        data.vertexCount = static_cast<GLsizei>(mesh->vertices.size());
    }

    // Vertex attribute pointers

    // Positions (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);

    // Normals (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(5 * sizeof(float)));

    // UVs (location = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));

    // Tangents (location = 3)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));

    // Bitangents (location = 4)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));

    // Unbind VAO
    glBindVertexArray(0);

    // Find the first available spot in the mesh data vector
    bool inserted = false;
    for (size_t i = 0; i < m_meshData.size(); ++i) {
        if (m_meshData[i].VAO == 0) {
            m_meshData[i] = data;
            mesh->id = i;
            inserted = true;
            break;
        }
    }

    // If all slots were full, append to end
    if (!inserted) {
        m_meshData.push_back(data);
        mesh->id = m_meshData.size() - 1;
    }

    // Clear CPU-side mesh data if possible
    if (isStatic) {
        mesh->clearData();
    }
}

void Renderer::deleteMeshBuffer(const Mesh* mesh) {
    size_t index = mesh->id;
    if (index >= m_meshData.size() || m_meshData[index].VAO == 0) {
        std::cerr << "[Error] Renderer::deleteMeshBuffer: Mesh buffer id not found!\n";
        return;
    }

    MeshData& data = m_meshData[index];

    // Delete mesh resources
    if (data.VAO) {
        glDeleteVertexArrays(1, &data.VAO);
        data.VAO = 0;
    }
    if (data.VBO) {
        glDeleteBuffers(1, &data.VBO);
        data.VBO = 0;
    }
    if (data.EBO) {
        glDeleteBuffers(1, &data.EBO);
        data.EBO = 0;
    }

    // Reset mesh at index
    m_meshData[index] = MeshData{};
}

/*
 * Screen Quad Initialization and Drawing
 */
void Renderer::initScreenQuad() {
    // Vertices for a screen-aligned quad (NDC space)
    float quadVertices[] = {
        // positions        // uvs
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f
    };

    unsigned int quadIndices[] = {
        0, 1, 2,
        2, 3, 0
    };

    // Generate and bind VAO
    glGenVertexArrays(1, &m_quadVAO);
    glBindVertexArray(m_quadVAO);

    // Generate and bind VBO
    unsigned int VBO, EBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    // Generate and bind EBO
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), &quadIndices, GL_STATIC_DRAW);

    // Position attribute (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    // UV attribute (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    // Unbind VAO
    glBindVertexArray(0);
}

void Renderer::drawScreenQuad() {
    glBindVertexArray(m_quadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

/*
* TODO: Render Passes and GBuffer code below (will be abstracted later)
*/

void Renderer::resizeGBuffer(int width, int height) {
    m_width = width;
    m_height = height;

    // Resize the G-buffer (and in the future, all framebuffers)
    m_gBuffer->resize(width, height);
    if (m_camera) {
        m_camera->setAspectRatio(static_cast<float>(width), static_cast<float>(height));
    }
}

/*
 * Render Passes
 */
void Renderer::geometryPass(entt::registry& registry, const glm::mat4& view) {
    // Bind G-buffer framebuffer
    m_gBuffer->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use Geometry Pass Shader
    m_gBufferShader.use();
    m_gBufferShader.setMat4("u_View", view);
    m_gBufferShader.setMat4("u_Projection", m_camera->getProjectionMatrix());

    // Loop through all entities with Mesh and ModelMatrix components
    // This is bad, but for now query for meshes in the individual passes
    auto viewMesh = registry.view<Mesh*, ModelMatrix>();
    for (auto entity : viewMesh) {
        const auto& mesh = registry.get<Mesh*>(entity);
        // Skip forward rendering materials
        if (!mesh->material->isDeferred) {
            continue;
        }

        const auto& modelMatrix = registry.get<ModelMatrix>(entity);
        m_gBufferShader.setMat4("u_Model", modelMatrix.matrix);

        mesh->material->bind(&m_gBufferShader);
        draw(mesh);
    }

    // Copy depth buffer from G-buffer to default framebuffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);  // Default framebuffer (the screen)
    glBlitFramebuffer(
        0, 0, m_width, m_height,
        0, 0, m_width, m_height,
        GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    m_gBuffer->unbind();
}

void Renderer::lightPass(entt::registry& registry) {
    // Bind default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Screen-space calculations; disable depth test
    glDisable(GL_DEPTH_TEST);

    // Enable additive blending for light accumulation
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    // Use the light pass shader
    m_lightPassShader.use();

    // Set camera position uniform
    m_lightPassShader.setVec3("u_CameraPosition", m_camera->getPosition());

    // Collect light data from ECS
    auto lightView = registry.view<Light, Position>();
    std::vector<PointLight> lights;
    // lights.reserve(lightView.size());

    for (auto entity : lightView) {
        const auto& lightComponent = lightView.get<Light>(entity);
        const auto& positionComponent = lightView.get<Position>(entity);

        // Only process point lights
        if (lightComponent.type != LightType::Point) {
            continue;
        }

        PointLight lightData;
        lightData.position = glm::vec4(positionComponent.position, 1.0f);
        lightData.color = glm::vec4(lightComponent.color, 1.0f);
        lightData.intensity = glm::vec4(lightComponent.intensity, 0.0f, 0.0f, 0.0f);

        lights.push_back(lightData);
    }

    // Update SSBO with light data
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lightSSBO);

    // Allocate buffer if needed
    static size_t currentBufferSize = 0;
    size_t requiredBufferSize = lights.size() * sizeof(PointLight);

    if (requiredBufferSize > currentBufferSize) {
        glBufferData(GL_SHADER_STORAGE_BUFFER, requiredBufferSize, lights.data(), GL_DYNAMIC_DRAW);
        currentBufferSize = requiredBufferSize;
    } else {
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, requiredBufferSize, lights.data());
    }

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_lightSSBO);

    // Set the number of lights
    m_lightPassShader.setInt("numLights", static_cast<int>(lights.size()));

    // Bind G-buffer textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getColorAttachment(0)); // Position
    m_lightPassShader.setInt("gPosition", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getColorAttachment(1)); // Normal
    m_lightPassShader.setInt("gNormal", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getColorAttachment(2)); // Albedo
    m_lightPassShader.setInt("gAlbedo", 2);

    // Draw full-screen quad
    drawScreenQuad();

    // Disable blending after lighting pass
    glDisable(GL_BLEND);

    // Re-enable depth testing for subsequent passes
    glEnable(GL_DEPTH_TEST);
    exit(0);
}

void Renderer::forwardPass(entt::registry& registry, const glm::mat4& view) {
    // Enable depth testing for forward pass
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    auto viewMesh = registry.view<Mesh*, ModelMatrix>();
    for (auto entity : viewMesh) {
        const auto& mesh = registry.get<Mesh*>(entity);
        const auto& modelMatrix = registry.get<ModelMatrix>(entity);

        // Skip deferred rendering materials
        if (mesh->material->isDeferred) {
            continue;
        }

        Shader* shader = mesh->material->shader;
        shader->use();

        // Set transformation matrices
        shader->setMat4("u_View", view);
        shader->setMat4("u_Projection", m_camera->getProjectionMatrix());
        shader->setMat4("u_Model", modelMatrix.matrix);

        mesh->material->bind();
        draw(mesh);
    }
}

/*
* Skybox
*/
void Renderer::skyboxPass(const glm::mat4& view, const glm::mat4& projection) {
    // Change depth function so the skybox renders behind everything
    glDepthFunc(GL_LEQUAL);

    // Disable depth writing so the skybox doesn't overwrite depth buffer values
    glDepthMask(GL_FALSE);

    // Use skybox shader
    m_skyboxShader.use();

    // Remove translation from the view matrix for the skybox
    glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));  // Remove translation
    m_skyboxShader.setMat4("view", viewNoTranslation);
    m_skyboxShader.setMat4("projection", projection);

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
}

void Renderer::initSkybox(const std::vector<std::string>& faces) {
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
}

/*
 * G-Buffer Debugging
 */
void Renderer::debugGBufferPass(const Shader& debugShader, int debugMode) {
    // Bind default framebuffer for debugging
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use the debug shader
    debugShader.use();

    // Set uniform samplers for G-buffer textures
    debugShader.setInt("gPosition", 0);
    debugShader.setInt("gNormal", 1);
    debugShader.setInt("gAlbedo", 2);
    debugShader.setInt("gDepth", 3);

    // Set debug mode (defines what to visualize)
    debugShader.setInt("debugMode", debugMode);

    // Bind G-buffer textures using the Framebuffer class
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getColorAttachment(0)); // Position

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getColorAttachment(1)); // Normal

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getColorAttachment(2)); // Albedo

    // Bind Depth Texture
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getDepthAttachment()); // Depth

    // Bind additional G-buffer textures if any (NUM_ATTACHMENTS > 4)
    for (int i = 4; i < NUM_ATTACHMENTS; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_gBuffer->getColorAttachment(i));
        debugShader.setInt("gExtraTexture" + std::to_string(i - 3), i);
    }

    // Draw the screen-aligned quad to visualize the debug information
    drawScreenQuad();

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
