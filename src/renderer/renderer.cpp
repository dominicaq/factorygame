#include "renderer.h"
#include <iostream>

Renderer::Renderer() {}

Renderer::~Renderer() {
    // Cleanup allocated buffers for each mesh
    for (auto& entry : m_meshBuffers) {
        const MeshData& data = entry.second;
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
}

void Renderer::init() {
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Optionally disable face culling for testing
    // glDisable(GL_CULL_FACE);
}

void Renderer::setupMesh(const Mesh* mesh) {
    // Check if the mesh has already been set up
    if (m_meshBuffers.find(mesh) != m_meshBuffers.end()) {
        return;
    }

    // Generate and bind VAO, VBO, and EBO for the mesh
    MeshData data;
    glGenVertexArrays(1, &data.VAO);
    glGenBuffers(1, &data.VBO);
    glGenBuffers(1, &data.EBO);

    glBindVertexArray(data.VAO);

    // Bind and fill vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, data.VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh->vertices.size() * sizeof(glm::vec3), &mesh->vertices[0], GL_STATIC_DRAW);

    // Bind and fill element (indices) buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size() * sizeof(unsigned int), &mesh->indices[0], GL_STATIC_DRAW);

    // Set vertex attribute pointers (only positions for now)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindVertexArray(0); // Unbind VAO for now

    // Store the mesh data in the map
    m_meshBuffers[mesh] = data;
}

void Renderer::render(const Mesh* mesh, const Shader& shader) {
    // Set up the mesh if it hasn't been set up yet
    if (m_meshBuffers.find(mesh) == m_meshBuffers.end()) {
        setupMesh(mesh);
    }

    // Get the VAO for the target mesh
    const MeshData& data = m_meshBuffers[mesh];

    // Activate the shader
    shader.use();

    // Bind the VAO (the mesh's vertex array)
    glBindVertexArray(data.VAO);

    // Render the mesh using the indices stored in EBO
    glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_INT, 0);

    // Unbind VAO after rendering
    glBindVertexArray(0);
}
