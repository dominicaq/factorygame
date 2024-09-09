#include "renderer.h"
#include <iostream>

Renderer::Renderer() : VAO(0), VBO(0), EBO(0) {}

Renderer::~Renderer() {
    // Cleanup any allocated buffers
    if (VAO) {
        glDeleteVertexArrays(1, &VAO);
    }

    if (VBO) {
        glDeleteBuffers(1, &VBO);
    }

    if (EBO) {
        glDeleteBuffers(1, &EBO);
    }
}

void Renderer::init() {
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Optionally disable face culling for testing
    glDisable(GL_CULL_FACE);
}

void Renderer::setupMesh(const Mesh* mesh) {
    // If already setup, don't do it again
    if (VAO != 0) {
        return;
    }

    // Generate and bind VAO, VBO, and EBO for the mesh
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Bind and fill vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh->vertices.size() * sizeof(glm::vec3), &mesh->vertices[0], GL_STATIC_DRAW);

    // Bind and fill element (indices) buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size() * sizeof(unsigned int), &mesh->indices[0], GL_STATIC_DRAW);

    // Set vertex attribute pointers (only positions for now)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindVertexArray(0); // Unbind VAO for now
}

void Renderer::render(const Mesh* mesh, const Shader& shader) {
    // Set up the mesh if it hasn't been set up yet
    if (VAO == 0 && mesh != nullptr) {
        setupMesh(mesh);
    }

    // Activate the shader
    shader.use();

    // Bind the VAO (the mesh's vertex array)
    glBindVertexArray(VAO);

    // Render the mesh using the indices stored in EBO
    glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_INT, 0);

    // Unbind VAO after rendering
    glBindVertexArray(0);
}
