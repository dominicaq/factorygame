#include "renderer.h"
#include <iostream>

Renderer::Renderer(unsigned int width, unsigned int height) {
    initOpenGLState();
    initGBuffer(width, height);
}

Renderer::~Renderer() {
    // Clean up allocated mesh buffers on the GPU
    for (const auto& entry : m_meshBuffers) {
        deleteMeshBuffer(entry.first);
    }
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

void Renderer::initGBuffer(unsigned int width, unsigned int height) {
    // Create the G-buffer FBO
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    // Position color buffer
    glActiveTexture(GL_TEXTURE0);  // Explicitly activate texture unit 0
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    // Normal color buffer
    glActiveTexture(GL_TEXTURE1);  // Explicitly activate texture unit 1
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    // Albedo color buffer
    glActiveTexture(GL_TEXTURE2);  // Explicitly activate texture unit 2
    glGenTextures(1, &gAlbedo);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

    // Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    // Create and attach depth buffer (renderbuffer)
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    // Check framebuffer status
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer not complete!" << std::endl;
    }

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::performGeometryPass(const Shader& geometryShader) {
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render all the meshes into the G-buffer
    for (const auto& entry : m_meshBuffers) {
        const Mesh* mesh = entry.first;
        draw(mesh, geometryShader);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::draw(const Mesh* mesh, const Shader& shader) {
    // Bind the shader to this draw call
    shader.use();

    // Load and bind the mesh buffer
    const MeshData& targetMesh = m_meshBuffers[mesh];
    glBindVertexArray(targetMesh.VAO);

    // Check if indices are available
    if (!mesh->indices.empty()) {
        // Render the mesh using indices
        GLsizei eboSize = static_cast<GLsizei>(mesh->indices.size());
        glDrawElements(GL_TRIANGLES, eboSize, GL_UNSIGNED_INT, 0);
    } else {
        // Render the mesh using vertex array without indices
        GLsizei vertexCount = static_cast<GLsizei>(mesh->vertices.size());
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }

    // Unbind VAO after rendering
    glBindVertexArray(0);
}

void Renderer::initMeshBuffers(const Mesh* mesh) {
    if (m_meshBuffers.find(mesh) != m_meshBuffers.end()) {
        return;
    }

    MeshData data;
    glGenVertexArrays(1, &data.VAO);
    glBindVertexArray(data.VAO);

    std::vector<float> bufferData;
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

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
    m_meshBuffers[mesh] = data;
}

void Renderer::deleteMeshBuffer(const Mesh* mesh) {
    auto it = m_meshBuffers.find(mesh);
    if (it != m_meshBuffers.end()) {
        const MeshData& target = it->second;
        if (target.VAO) glDeleteVertexArrays(1, &target.VAO);
        if (target.VBO) glDeleteBuffers(1, &target.VBO);
        if (target.EBO) glDeleteBuffers(1, &target.EBO);
        m_meshBuffers.erase(it);
    } else {
        std::cerr << "ERROR: Mesh buffer not found.\n";
    }
}

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
