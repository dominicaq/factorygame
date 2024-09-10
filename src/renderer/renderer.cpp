#include "renderer.h"
#include <iostream>

Renderer::Renderer() {
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Optionally disable face culling for testing
    // glDisable(GL_CULL_FACE);
}

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

    // Interleaved vertex buffer (positions, uvs, normals)
    std::vector<float> bufferData;
    for (size_t i = 0; i < mesh->vertices.size(); ++i) {
        // Position
        bufferData.push_back(mesh->vertices[i].x);
        bufferData.push_back(mesh->vertices[i].y);
        bufferData.push_back(mesh->vertices[i].z);

        // Texture coordinates (UVs)
        bufferData.push_back(mesh->uvs[i].x);
        bufferData.push_back(mesh->uvs[i].y);

        // Normals
        bufferData.push_back(mesh->normals[i].x);
        bufferData.push_back(mesh->normals[i].y);
        bufferData.push_back(mesh->normals[i].z);
    }

    // Bind and fill vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, data.VBO);
    glBufferData(GL_ARRAY_BUFFER, bufferData.size() * sizeof(float), bufferData.data(), GL_STATIC_DRAW);

    // Bind and fill element (indices) buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size() * sizeof(unsigned int), mesh->indices.data(), GL_STATIC_DRAW);

    // Set vertex attribute pointers
    // Positions (location = 0, 3 floats)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    // UVs (location = 1, 2 floats)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    // Normals (location = 2, 3 floats)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));

    // Unbind VAO for now
    glBindVertexArray(0);

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
