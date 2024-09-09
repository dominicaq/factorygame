#ifndef RENDERER_H
#define RENDERER_H

#include "../resources/mesh.h"
#include "shader.h"
#include <glad/glad.h>

class Renderer {
public:
    Renderer();
    ~Renderer();

    // Initialize any required OpenGL state
    void init();

    // Render a single mesh with target shader
    void render(const Mesh& mesh, const Shader& shader);

private:
    // OpenGL mesh data
    GLuint VAO, VBO, EBO;
    // Set up the VAO, VBO, and EBO for a mesh
    void setupMesh(const Mesh& mesh);
};

#endif // RENDERER_H
