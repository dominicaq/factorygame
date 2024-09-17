#include "renderer.h"
#include <iostream>

#define NUM_ATTACHMENTS 3

Renderer::Renderer(int width, int height) {
    initOpenGLState();

    initGBuffer(width, height);

    // Geometry Pass Shader (Expecting this to be fixed, so will be created here)
    std::string gBufferVertexPath = ASSET_DIR "shaders/deferred/gbuff.vs";
    std::string gBufferFragmentPath = ASSET_DIR "shaders/deferred/gbuff.fs";
    if (!m_gBufferShader.load(gBufferVertexPath, gBufferFragmentPath)) {
        std::cerr << "Failed to create gBufferShader!\n";
    }

    // Light Pass Shader (Expecting this to be fixed, so will be created here)
    std::string lightVertexPath = ASSET_DIR "shaders/deferred/lightpass.vs";
    std::string lightFragmentPath = ASSET_DIR "shaders/deferred/lightpass.fs";
    if (!m_lightPassShader.load(lightVertexPath, lightFragmentPath)) {
        std::cerr << "Failed to create light shader!\n";
    }

    // Create VAO for screen quad
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

    // Culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
}

/*
* Forward Rendering
*/


/*
* G-buffer Management
*/
void Renderer::initGBuffer(int width, int height) {
    // Create the G-buffer FBO
    glGenFramebuffers(1, &m_gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);

    // Resize the vector to store NUM_ATTACHMENTS
    m_gTextures.resize(NUM_ATTACHMENTS);

    // Initialize position texture
    glGenTextures(1, &m_gTextures[0]);
    glBindTexture(GL_TEXTURE_2D, m_gTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_gTextures[0], 0); // Position

    // Initialize normal texture
    glGenTextures(1, &m_gTextures[1]);
    glBindTexture(GL_TEXTURE_2D, m_gTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_gTextures[1], 0); // Normal

    // Initialize albedo texture
    glGenTextures(1, &m_gTextures[2]);
    glBindTexture(GL_TEXTURE_2D, m_gTextures[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_gTextures[2], 0); // Albedo

    // Initialize any additional G-buffer textures
    for (int i = 3; i < NUM_ATTACHMENTS; ++i) {
        glGenTextures(1, &m_gTextures[i]);
        glBindTexture(GL_TEXTURE_2D, m_gTextures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3 + (i - 3), GL_TEXTURE_2D, m_gTextures[i], 0);
    }

    // Set the list of draw buffers
    GLuint attachments[NUM_ATTACHMENTS];
    for (int i = 0; i < NUM_ATTACHMENTS; ++i) {
        attachments[i] = GL_COLOR_ATTACHMENT0 + i;
    }
    glDrawBuffers(NUM_ATTACHMENTS, attachments);

    // Create and attach depth buffer
    glGenRenderbuffers(1, &m_rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rboDepth);

    // Check framebuffer status
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer not complete!\n";
    }

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::resizeGBuffer(int width, int height) {
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
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
            mesh->material->albedoMap->bind(0);
            m_gBufferShader.setInt("u_AlbedoMap", 0);

            if (mesh->material->normalMap) {
                mesh->material->normalMap->bind(1);
                m_gBufferShader.setInt("u_NormalMap", 1);
                m_gBufferShader.setBool("u_HasNormalMap", true);
            } else {
                m_gBufferShader.setBool("u_HasNormalMap", false);
            }
        }

        draw(mesh);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::setupMaterial(Shader* shader, const Material* material) {
    // Bind the albedo map
    shader->setInt("u_AlbedoMap", 0);
    material->albedoMap->bind(0);
    shader->setVec3("u_AlbedoColor", material->albedoColor);

    // Bind normal map if available
    // shader->setBool("u_HasNormalMap", material->normalMap != nullptr);
    // if (material->normalMap) {
    //     material->normalMap->bind(1);
    //     shader->setInt("u_NormalMap", 1);
    // }

    // // Bind specular map if available
    // shader->setBool("u_HasSpecularMap", material->specularMap != nullptr);
    // if (material->specularMap) {
    //     material->specularMap->bind(2);
    //     shader->setInt("u_SpecularMap", 2);
    // }
}

void Renderer::setupLight(Shader* shader, const LightSystem& lightSystem, int index) {
    const glm::vec3& lightPos = lightSystem.positions[index];
    const glm::vec3& lightColor = lightSystem.colors[index];
    float lightIntensity = lightSystem.lightIntensities[index];
    bool isDirectional = lightSystem.directionalFlags[index];
    const glm::vec3& lightDir = lightSystem.directions[index];

    // Pass light properties to the shader
    shader->setVec3("u_LightPosition", lightPos);
    shader->setVec3("u_LightColor", lightColor);
    shader->setFloat("u_LightIntensity", lightIntensity);
    // shader->setBool("u_IsDirectional", isDirectional);

    if (isDirectional) {
        shader->setVec3("u_LightDirection", lightDir);
    }
}

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

    // Handle additional G-buffer textures
    for (int i = 3; i < NUM_ATTACHMENTS; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_gTextures[i]);
        shader->setInt("gExtraTexture" + std::to_string(i - 3), i);
    }
}

void Renderer::forwardPass(const std::vector<Mesh*>& meshes,
                           const std::vector<Transform>& transforms,
                           const glm::mat4& view,
                           const glm::mat4& projection) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (size_t i = 0; i < meshes.size(); ++i) {
        const Mesh* mesh = meshes[i];
        const Transform& transform = transforms[i];

        Shader* shader = mesh->material->shader;
        if (!shader) {
            std::cerr << "ERROR::RENDERER::FORWARD_PASS::Mesh does not have a shader!\n";
            continue;
        }

        // Use the mesh's shader and set matrices
        shader->use();
        shader->setMat4("u_View", view);
        shader->setMat4("u_Projection", projection);
        shader->setMat4("u_Model", transform.getModelMatrix());

        // Setup material
        if (mesh->material) {
            setupMaterial(shader, mesh->material);
        }

        // Draw the mesh
        draw(mesh);
    }
}

void Renderer::lightPass(const glm::vec3& cameraPosition, const LightSystem& lightSystem) {
    // Bind the default framebuffer to render the final image
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use the light pass shader
    m_lightPassShader.use();

    // Set camera position (for specular calculation)
    m_lightPassShader.setVec3("u_CameraPosition", cameraPosition);

    // Setup light (assuming one light for now, can be extended)
    setupLight(&m_lightPassShader, lightSystem, 0);

    // Setup G-buffer textures
    setupGBufferTextures(&m_lightPassShader);

    // Draw the screen-aligned quad (for full-screen lighting)
    drawScreenQuad();

    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

    // Bind buffers
    glBindVertexArray(data.VAO);

    if (data.EBO != 0) {
        glDrawElements(GL_TRIANGLES, data.indexCount, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, data.vertexCount);
    }

    // Unbind buffers
    glBindVertexArray(0);
}

void Renderer::initMeshBuffers(Mesh* mesh, bool isStatic) {
    if (mesh->uvs.size() == 0 || mesh->normals.size() == 0 || mesh->tangents.size() == 0 || mesh->bitangents.size() == 0) {
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

    // Build mesh buffer data (14 floats per vertex: 3 for position, 2 for UVs, 3 for normals, 3 for tangents, 3 for bitangents)
    std::vector<float> bufferData;
    size_t numVertices = mesh->vertices.size();
    bufferData.resize(numVertices * 14);

    float* ptr = bufferData.data();
    for (size_t i = 0; i < numVertices; ++i) {
        // Positions
        *ptr++ = mesh->vertices[i].x;
        *ptr++ = mesh->vertices[i].y;
        *ptr++ = mesh->vertices[i].z;
        // UVs
        *ptr++ = mesh->uvs[i].x;
        *ptr++ = mesh->uvs[i].y;
        // Normals
        *ptr++ = mesh->normals[i].x;
        *ptr++ = mesh->normals[i].y;
        *ptr++ = mesh->normals[i].z;
        // Tangents
        *ptr++ = mesh->tangents[i].x;
        *ptr++ = mesh->tangents[i].y;
        *ptr++ = mesh->tangents[i].z;
        // Bitangents
        *ptr++ = mesh->bitangents[i].x;
        *ptr++ = mesh->bitangents[i].y;
        *ptr++ = mesh->bitangents[i].z;
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

    // UVs (location = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));

    // Normals (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(5 * sizeof(float)));

    // Tangents (location = 3)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));

    // Bitangents (location = 4)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));

    // Unbind buffers
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

    // Reset mesh at index (this will leave a hole in the array)
    m_meshData[index] = MeshData{};
}

/*
* Screen quad deferred rendering target output
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

    // Generate and bind EBO (Element Buffer Object for indices)
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
    // Bind core G-buffer textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_gTextures[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_gTextures[1]);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_gTextures[2]);

    // Use the debug shader
    debugShader.use();
    debugShader.setInt("gPosition", 0);
    debugShader.setInt("gNormal", 1);
    debugShader.setInt("gAlbedo", 2);
    debugShader.setInt("debugMode", debugMode);

    // Bind any additional G-buffer textures if they exist
    for (int i = 3; i < NUM_ATTACHMENTS; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_gTextures[i]);
        debugShader.setInt("gExtraTexture" + std::to_string(i - 3), i);
    }

    // Draw the quad (screen aligned)
    drawScreenQuad();
}
