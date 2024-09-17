#include "renderer.h"
#include <iostream>

// Define the number of G-buffer attachments
#define NUM_ATTACHMENTS 4

Renderer::Renderer(int width, int height) : m_width(width), m_height(height) {
    initOpenGLState();
    initGBuffer(width, height);

    // Load Geometry Pass Shader
    std::string gBufferVertexPath = ASSET_DIR "shaders/deferred/gbuff.vs";
    std::string gBufferFragmentPath = ASSET_DIR "shaders/deferred/gbuff.fs";
    if (!m_gBufferShader.load(gBufferVertexPath, gBufferFragmentPath)) {
        std::cerr << "Failed to create gBufferShader!\n";
    }

    // Load Light Pass Shader
    std::string lightVertexPath = ASSET_DIR "shaders/deferred/lightpass.vs";
    std::string lightFragmentPath = ASSET_DIR "shaders/deferred/lightpass.fs";
    if (!m_lightPassShader.load(lightVertexPath, lightFragmentPath)) {
        std::cerr << "Failed to create lightPassShader!\n";
    }

    // Initialize Screen Quad for Deferred Passes
    initScreenQuad();
}

Renderer::~Renderer() {
    // Clean up G-buffer
    glDeleteFramebuffers(1, &m_gBuffer);
    for (auto& texture : m_gTextures) {
        glDeleteTextures(1, &texture);
    }
    glDeleteRenderbuffers(1, &m_rboDepth);

    // Clean up mesh buffers
    for (auto& data : m_meshData) {
        if (data.VAO) glDeleteVertexArrays(1, &data.VAO);
        if (data.VBO) glDeleteBuffers(1, &data.VBO);
        if (data.EBO) glDeleteBuffers(1, &data.EBO);
    }

    // Clean up quad
    if (m_quadVAO) glDeleteVertexArrays(1, &m_quadVAO);
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
 * G-buffer Management
 */
void Renderer::initGBuffer(int width, int height) {
    m_width = width;
    m_height = height;

    // Create the G-buffer Framebuffer Object (FBO)
    glGenFramebuffers(1, &m_gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);

    // Resize the vector to store NUM_ATTACHMENTS
    m_gTextures.resize(NUM_ATTACHMENTS);

    // 1. Position Texture
    glGenTextures(1, &m_gTextures[0]);
    glBindTexture(GL_TEXTURE_2D, m_gTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_gTextures[0], 0); // Position

    // 2. Normal Texture
    glGenTextures(1, &m_gTextures[1]);
    glBindTexture(GL_TEXTURE_2D, m_gTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_gTextures[1], 0); // Normal

    // 3. Albedo Texture
    glGenTextures(1, &m_gTextures[2]);
    glBindTexture(GL_TEXTURE_2D, m_gTextures[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_gTextures[2], 0); // Albedo

    // 4. Depth Texture
    glGenTextures(1, &m_gTextures[3]);
    glBindTexture(GL_TEXTURE_2D, m_gTextures[3]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    // Set border color for depth texture
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // Depth
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_gTextures[3], 0);

    // Specify the list of color attachments to draw to
    std::vector<GLenum> attachments;
    for (int i = 0; i < NUM_ATTACHMENTS - 1; ++i) { // Exclude depth
        attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
    }
    glDrawBuffers(NUM_ATTACHMENTS - 1, attachments.data());

    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "G-buffer Framebuffer is not complete!\n";
    }

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::resizeGBuffer(int width, int height) {
    // Update stored width and height
    m_width = width;
    m_height = height;

    // Delete existing G-buffer textures
    for (unsigned int texture : m_gTextures) {
        glDeleteTextures(1, &texture);
    }

    // Delete depth renderbuffer
    glDeleteRenderbuffers(1, &m_rboDepth);

    // Re-initialize the G-buffer with the new size
    initGBuffer(width, height);
}

/*
 * Render Passes
 */
void Renderer::geometryPass(const std::vector<Mesh*>& meshes,
                            const std::vector<Transform>& transforms,
                            const glm::mat4& view,
                            const glm::mat4& projection) {
    // Bind G-buffer framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use Geometry Pass Shader
    m_gBufferShader.use();
    m_gBufferShader.setMat4("u_View", view);
    m_gBufferShader.setMat4("u_Projection", projection);

    for (size_t i = 0; i < meshes.size(); ++i) {
        const Mesh* mesh = meshes[i];
        const Transform& transform = transforms[i];

        glm::mat4 model = transform.getModelMatrix();
        m_gBufferShader.setMat4("u_Model", model);

        // Set material properties
        if (mesh->material) {
            m_gBufferShader.setVec3("u_AlbedoColor", mesh->material->albedoColor);

            // Bind Albedo Map
            if (mesh->material->albedoMap) {
                mesh->material->albedoMap->bind(0);
                m_gBufferShader.setInt("u_AlbedoMap", 0);
                // m_gBufferShader.setBool("u_HasAlbedoMap", true);
            } else {
                // m_gBufferShader.setBool("u_HasAlbedoMap", false);
            }

            // Bind Normal Map
            if (mesh->material->normalMap) {
                mesh->material->normalMap->bind(1);
                m_gBufferShader.setInt("u_NormalMap", 1);
                m_gBufferShader.setBool("u_HasNormalMap", true);
            } else {
                m_gBufferShader.setBool("u_HasNormalMap", false);
            }

            // Bind Specular Map (if applicable)
            // if (mesh->material->specularMap) {
            //     mesh->material->specularMap->bind(2);
            //     m_gBufferShader.setInt("u_SpecularMap", 2);
            //     m_gBufferShader.setBool("u_HasSpecularMap", true);
            // } else {
            //     m_gBufferShader.setBool("u_HasSpecularMap", false);
            // }
        }

        draw(mesh);
    }

    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::lightPass(const glm::vec3& cameraPosition, const LightSystem& lightSystem) {
    // Bind default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use Light Pass Shader
    m_lightPassShader.use();

    // Set camera position
    m_lightPassShader.setVec3("u_CameraPosition", cameraPosition);

    // Set the number of lights
    m_lightPassShader.setInt("numLights", lightSystem.size);

    // Pass each light's data to the shader
    for (int i = 0; i < lightSystem.size; ++i) {
        std::string indexStr = "[" + std::to_string(i) + "]";

        m_lightPassShader.setVec3("lights" + indexStr + ".position", lightSystem.positions[i]);
        m_lightPassShader.setVec3("lights" + indexStr + ".color", lightSystem.colors[i]);
        m_lightPassShader.setFloat("lights" + indexStr + ".intensity", lightSystem.lightIntensities[i]);
        m_lightPassShader.setBool("lights" + indexStr + ".isDirectional", lightSystem.directionalFlags[i]);

        if (lightSystem.directionalFlags[i]) {
            m_lightPassShader.setVec3("lights" + indexStr + ".direction", lightSystem.directions[i]);
        }
    }

    // Setup G-buffer textures for lighting
    setupGBufferTextures(&m_lightPassShader);

    // Enable additive blending for lighting accumulation
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    // Draw the screen-aligned quad for lighting
    drawScreenQuad();

    // Disable blending after lighting pass
    glDisable(GL_BLEND);
}

void Renderer::forwardPass(const std::vector<Mesh*>& meshes,
                           const std::vector<Transform>& transforms,
                           const glm::mat4& view,
                           const glm::mat4& projection) {
    // Copy depth buffer from G-buffer to default framebuffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_gBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, m_width, m_height,
                      0, 0, m_width, m_height,
                      GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Enable depth testing for forward pass
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Disable face culling for transparent objects (if any)
    // glDisable(GL_CULL_FACE);

    // Iterate over all forward pass meshes
    for (size_t i = 0; i < meshes.size(); ++i) {
        const Mesh* mesh = meshes[i];
        const Transform& transform = transforms[i];

        Shader* shader = mesh->material->shader;
        if (!shader) {
            std::cerr << "ERROR::FORWARD_PASS::Mesh does not have a shader!\n";
            continue;
        }

        // Use the forward shader
        shader->use();

        // Set transformation matrices
        shader->setMat4("u_View", view);
        shader->setMat4("u_Projection", projection);
        shader->setMat4("u_Model", transform.getModelMatrix());

        // Setup material properties and bind textures
        setupMaterial(shader, mesh->material);

        // Draw the mesh
        draw(mesh);
    }

    // Re-enable face culling if it was disabled
    // glEnable(GL_CULL_FACE);
}

/*
 * Material Setup for Forward Pass
 */
void Renderer::setupMaterial(Shader* shader, const Material* material) {
    // Set Albedo Color
    shader->setVec3("u_AlbedoColor", material->albedoColor);

    // Bind Albedo Map
    if (material->albedoMap) {
        glActiveTexture(GL_TEXTURE0);
        material->albedoMap->bind(0);
        shader->setInt("u_AlbedoMap", 0);
        // shader->setBool("u_HasAlbedoMap", true);
    } else {
        // shader->setBool("u_HasAlbedoMap", false);
    }

    // Bind Normal Map
    if (material->normalMap) {
        glActiveTexture(GL_TEXTURE1);
        material->normalMap->bind(1);
        shader->setInt("u_NormalMap", 1);
        // shader->setBool("u_HasNormalMap", true);
    } else {
        // shader->setBool("u_HasNormalMap", false);
    }

    // Bind Specular Map
    // if (material->specularMap) {
    //     glActiveTexture(GL_TEXTURE2);
    //     material->specularMap->bind(2);
    //     shader->setInt("u_SpecularMap", 2);
    //     shader->setBool("u_HasSpecularMap", true);
    // } else {
    //     shader->setBool("u_HasSpecularMap", false);
    // }

    // Additional material properties can be set here
}

/*
 * G-buffer Texture Setup for Shaders
 */
void Renderer::setupGBufferTextures(Shader* shader) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_gTextures[0]);
    shader->setInt("gPosition", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_gTextures[1]);
    shader->setInt("gNormal", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_gTextures[2]);
    shader->setInt("gAlbedo", 2);

    // Bind additional G-buffer textures if any
    for (int i = 4; i < NUM_ATTACHMENTS; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_gTextures[i]);
        shader->setInt("gExtraTexture" + std::to_string(i - 3), i);
    }
}

/*
 * Mesh Management
 */
void Renderer::draw(const Mesh* mesh) {
    size_t index = mesh->id;
    if (index >= m_meshData.size() || m_meshData[index].VAO == 0) {
        std::cerr << "ERROR::RENDERER::DRAW::Mesh buffer not found.\n";
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
    if (mesh->uvs.empty() || mesh->normals.empty() || mesh->tangents.empty() || mesh->bitangents.empty()) {
        std::cerr << "ERROR::RENDERER::INIT_MESH_BUFFERS::UVs, normals, tangents, and bitangents must be provided for all vertices.\n";
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
        std::cerr << "ERROR::RENDERER::DELETE_MESH_BUFFER::Mesh buffer not found.\n";
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
 * G-Buffer Debugging
 */
void Renderer::debugGBuffer(const Shader& debugShader, int debugMode) {
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

    // Bind G-buffer textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_gTextures[0]); // Position

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_gTextures[1]); // Normal

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_gTextures[2]); // Albedo

    // Bind Depth Texture
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_gTextures[3]); // Depth

    // Bind additional G-buffer textures if any (NUM_ATTACHMENTS > 4)
    for (int i = 4; i < NUM_ATTACHMENTS; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_gTextures[i]);
        debugShader.setInt("gExtraTexture" + std::to_string(i - 3), i);
    }

    // Draw the screen-aligned quad to visualize the debug information
    drawScreenQuad();

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
