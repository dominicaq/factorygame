#include "renderer.h"
#include <iostream>

Renderer::Renderer(unsigned int width, unsigned int height) {
    initOpenGLState();
    initGBuffer(width, height);
}

Renderer::~Renderer() {
    // Iterate over the map and delete all mesh buffers
    for (auto it = m_meshBuffers.begin(); it != m_meshBuffers.end(); ) {
        const MeshData& target = it->second;

        // Delete VAO, VBO, and EBO if they exist
        if (target.VAO) {
            glDeleteVertexArrays(1, &target.VAO);
        }
        if (target.VBO) {
            glDeleteBuffers(1, &target.VBO);
        }
        if (target.EBO) {
            glDeleteBuffers(1, &target.EBO);
        }

        // Erase the mesh entry from the map and move the iterator forward
        it = m_meshBuffers.erase(it);
    }

    // Delete G-buffer resources
    glDeleteFramebuffers(1, &m_gBuffer);
    glDeleteTextures(1, &m_gPosition);
    glDeleteTextures(1, &m_gNormal);
    glDeleteTextures(1, &m_gAlbedo);
    glDeleteRenderbuffers(1, &m_rboDepth);
}

void Renderer::initOpenGLState() {
    glEnable(GL_DEPTH_TEST);

    // Culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Clear color
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

/*
* GBuffer management
*/
void Renderer::initGBuffer(unsigned int width, unsigned int height) {
    // Create the G-buffer FBO
    glGenFramebuffers(1, &m_gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);

    // Position color buffer
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &m_gPosition);
    glBindTexture(GL_TEXTURE_2D, m_gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_gPosition, 0);

    // Normal color buffer
    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &m_gNormal);
    glBindTexture(GL_TEXTURE_2D, m_gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_gNormal, 0);

    // Albedo color buffer
    glActiveTexture(GL_TEXTURE2);
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
        GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    // Create and attach depth buffer
    glGenRenderbuffers(1, &m_rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rboDepth);

    // Check framebuffer status
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::resizeGBuffer(unsigned int newWidth, unsigned int newHeight) {
    // Delete existing G-buffer textures and depth buffer
    glDeleteTextures(1, &m_gPosition);
    glDeleteTextures(1, &m_gNormal);
    glDeleteTextures(1, &m_gAlbedo);
    glDeleteRenderbuffers(1, &m_rboDepth);

    // Re-initialize the G-buffer with new size
    initGBuffer(newWidth, newHeight);
}

void Renderer::geometryPass(const Shader& geometryShader) {
    // Bind the gbuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);

    // Render all the meshes into the G-buffer
    for (const auto& entry : m_meshBuffers) {
        const Mesh* mesh = entry.first;

        draw(mesh, geometryShader);
    }

    // Unbind the framebuffer after the pass
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << error << std::endl;
    }
}

/*
* Mesh buffer management
*/
void Renderer::draw(const Mesh* mesh, const Shader& shader) {
    // shader.use();

    // Load and bind the mesh buffer
    const MeshData& targetMesh = m_meshBuffers.at(mesh);
    glBindVertexArray(targetMesh.VAO);

    if (!mesh->indices.empty()) {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(mesh->vertices.size()));
    }

    glBindVertexArray(0);
}

void Renderer::initMeshBuffers(const Mesh* mesh) {
    // Mesh buffer already created
    if (m_meshBuffers.find(mesh) != m_meshBuffers.end()) {
        return;
    }

    MeshData data;
    glGenVertexArrays(1, &data.VAO);
    glBindVertexArray(data.VAO);

    const std::vector<glm::vec3>* normalsToUse = &mesh->normals;
    std::vector<glm::vec3> generatedNormals(mesh->vertices.size(), glm::vec3(0.0f));
    if (mesh->normals.empty() && !mesh->indices.empty()) {
        generateNormals(mesh, generatedNormals, mesh->indices);
        normalsToUse = &generatedNormals;
    }

    const std::vector<glm::vec2>* uvsToUse = &mesh->uvs;
    std::vector<glm::vec2> generatedUVs;
    if (mesh->uvs.empty()) {
        generateUVs(mesh, generatedUVs);
        uvsToUse = &generatedUVs;
    }

    std::vector<float> bufferData;
    for (size_t i = 0; i < mesh->vertices.size(); ++i) {
        bufferData.push_back(mesh->vertices[i].x);
        bufferData.push_back(mesh->vertices[i].y);
        bufferData.push_back(mesh->vertices[i].z);
        bufferData.push_back((*uvsToUse)[i].x);
        bufferData.push_back((*uvsToUse)[i].y);
        bufferData.push_back((*normalsToUse)[i].x);
        bufferData.push_back((*normalsToUse)[i].y);
        bufferData.push_back((*normalsToUse)[i].z);
    }

    glGenBuffers(1, &data.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, data.VBO);
    glBufferData(GL_ARRAY_BUFFER, bufferData.size() * sizeof(float), bufferData.data(), GL_STATIC_DRAW);

    if (!mesh->indices.empty()) {
        glGenBuffers(1, &data.EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size() * sizeof(unsigned int), mesh->indices.data(), GL_STATIC_DRAW);
    }

    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    // UVs
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    // Normals
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));

    glBindVertexArray(0);
    m_meshBuffers[mesh] = data;
}

void Renderer::deleteMeshBuffer(const Mesh* mesh) {
    auto it = m_meshBuffers.find(mesh);
    if (it != m_meshBuffers.end()) {
        // Access MeshData directly from the iterator
        const MeshData& target = it->second;

        // Delete VAO, VBO, and EBO if they exist
        if (target.VAO) {
            glDeleteVertexArrays(1, &target.VAO);
        }
        if (target.VBO) {
            glDeleteBuffers(1, &target.VBO);
        }
        if (target.EBO) {
            glDeleteBuffers(1, &target.EBO);
        }

        // Erase the mesh entry from the map
        m_meshBuffers.erase(it);
    } else {
        std::cerr << "ERROR: Mesh buffer not found." << std::endl;
    }
}

/*
* Mesh data generators
*/
void Renderer::generateNormals(const Mesh* mesh, std::vector<glm::vec3>& normals, const std::vector<unsigned int>& indices) {
    normals.clear();
    normals.resize(mesh->vertices.size(), glm::vec3(0.0f));

    for (size_t i = 0; i < indices.size(); i += 3) {
        unsigned int index0 = indices[i];
        unsigned int index1 = indices[i + 1];
        unsigned int index2 = indices[i + 2];

        glm::vec3 v0 = mesh->vertices[index0];
        glm::vec3 v1 = mesh->vertices[index1];
        glm::vec3 v2 = mesh->vertices[index2];

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

        normals[index0] += faceNormal;
        normals[index1] += faceNormal;
        normals[index2] += faceNormal;
    }

    for (size_t i = 0; i < normals.size(); ++i) {
        normals[i] = glm::normalize(normals[i]);
    }
}

void Renderer::generateUVs(const Mesh* mesh, std::vector<glm::vec2>& uvs) {
    uvs.reserve(mesh->vertices.size());
    for (const auto& vertex : mesh->vertices) {
        float u = (vertex.x + 1.0f) * 0.5f;
        float v = (vertex.y + 1.0f) * 0.5f;
        uvs.push_back(glm::vec2(u, v));
    }
}
