#pragma once

#include <unordered_map>
#include <glad/glad.h>

#include "shader.h"
#include "../resources/mesh.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    void init();
    void setupMesh(const Mesh* mesh);
    void render(const Mesh* mesh, const Shader& shader);

private:
    struct MeshData {
        GLuint VAO;
        GLuint VBO;
        GLuint EBO;
    };

    std::unordered_map<const Mesh*, MeshData> m_meshBuffers;
};
