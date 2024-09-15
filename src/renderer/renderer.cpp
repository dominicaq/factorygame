#include "renderer.h"
#include <iostream>

Renderer::Renderer(int width, int height) {
    initOpenGLState();
    initGBuffer(width, height);
    initQuad();
}

Renderer::~Renderer() {
    // Clean up G-buffer
    glDeleteFramebuffers(1, &m_gBuffer);
    glDeleteTextures(1, &m_gPosition);
    glDeleteTextures(1, &m_gNormal);
    glDeleteTextures(1, &m_gAlbedo);
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
* G-buffer Management
*/
void Renderer::initGBuffer(int width, int height) {
    // Create the G-buffer FBO
    glGenFramebuffers(1, &m_gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);

    // Position color buffer
    glGenTextures(1, &m_gPosition);
    glBindTexture(GL_TEXTURE_2D, m_gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_gPosition, 0);

    // Normal color buffer
    glGenTextures(1, &m_gNormal);
    glBindTexture(GL_TEXTURE_2D, m_gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_gNormal, 0);

    // Albedo color buffer
    glGenTextures(1, &m_gAlbedo);
    glBindTexture(GL_TEXTURE_2D, m_gAlbedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_gAlbedo, 0);

    // Set the list of draw buffers
    GLuint attachments[3] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2
    };
    glDrawBuffers(3, attachments);

    // Create and attach depth buffer
    glGenRenderbuffers(1, &m_rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rboDepth);

    // Check framebuffer status with more detailed error output
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer not complete! Status: " << status << "\n";
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::resizeGBuffer(int width, int height) {
    // Delete existing G-buffer textures and depth buffer
    glDeleteTextures(1, &m_gPosition);
    glDeleteTextures(1, &m_gNormal);
    glDeleteTextures(1, &m_gAlbedo);
    glDeleteRenderbuffers(1, &m_rboDepth);

    // Re-initialize the G-buffer with new size
    initGBuffer(width, height);
}

void Renderer::geometryPass(const Shader& shader, const std::vector<Mesh*>& meshes, const std::vector<Transform>& transforms) {
    // Bind and clear framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.use();

    for (size_t i = 0; i < meshes.size(); ++i) {
        const Mesh* mesh = meshes[i];
        const Transform& transform = transforms[i];

        // Set model matrix
        glm::mat4 model = transform.getModelMatrix();
        shader.setMat4("u_Model", model);

        // Create Material map(s)
        if (mesh->material) {
            // Set the albedo color
            shader.setVec3("u_AlbedoColor", mesh->material->albedoColor);

            // Set the texture unit to 0 for the shader's sampler uniform
            shader.setInt("u_AlbedoTexture", 0);

            // Bind the albedo texture to texture unit 0
            if (mesh->material->albedoTexture) {
                mesh->material->albedoTexture->bind(0);
            }
        }

        // Draw the mesh
        draw(mesh);
    }

    // Unbind and clear framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

void Renderer::initMeshBuffers(Mesh* mesh) {
    if (mesh->uvs.size() == 0 || mesh->normals.size() == 0) {
        std::cerr << "ERROR::RENDERER::INIT_MESH_BUFFERS::UVs and normals must be provided for all vertices.\n";
        return;
    }

    // Assign an ID if not already assigned
    if (mesh->id == SIZE_MAX) {
        mesh->id = m_meshData.size();
    }

    MeshData data = {};
    glGenVertexArrays(1, &data.VAO);
    glBindVertexArray(data.VAO);

    // Build mesh buffer data (8 floats per vertex)
    std::vector<float> bufferData;
    size_t numVertices = mesh->vertices.size();
    bufferData.resize(numVertices * 8);

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

    // Vertex positions (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0));

    // Normals (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));

    // UVs (location = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

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

    // Clear CPU-side mesh data
    mesh->clearData();
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
* Deferred Rendering Quad
*/
void Renderer::initQuad() {
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

void Renderer::drawQuad() {
    glBindVertexArray(m_quadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

/*
* G-Buffer Debugging
*/
void Renderer::debugGBuffer(const Shader& debugShader, int debugMode) {
    // Bind the G-buffer textures to texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_gAlbedo);

    // Use the debug shader
    debugShader.use();
    debugShader.setInt("gPosition", 0);
    debugShader.setInt("gNormal", 1);
    debugShader.setInt("gAlbedo", 2);
    debugShader.setInt("debugMode", debugMode);

    // Draw the quad (screen aligned)
    drawQuad();
}
