#ifndef RENDERER_H
#define RENDERER_H

#include <glad/glad.h>
#include "shader.h"
#include "../resources/mesh.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    void init();
    void render(const Mesh* mesh, const Shader& shader);

private:
    void setupMesh(const Mesh* mesh);

    unsigned int VAO, VBO, EBO;
};

#endif // RENDERER_H
