#ifndef RENDERER_H
#define RENDERER_H

#include "shader.h"
#include "../components/transform.h"
#include "../components/mesh.h"
#include "../components/light.h"

#include <glad/glad.h>

#include <memory>
#include <vector>

/*
* The Renderer class is responsible for handling OpenGL rendering, including
* G-buffer management for deferred rendering and mesh rendering.
*/
class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();

    /*
    * Render mesh using its buffers
    */
    void draw(const Mesh* mesh);

    /*
    * Initialize and manage mesh buffers
    */
    void initMeshBuffers(Mesh* mesh, bool isStatic = true);
    void deleteMeshBuffer(const Mesh* mesh);

    /*
    * Forward rendering
    */
   void forwardPass(const std::vector<Mesh*>& meshes,
        const std::vector<Transform>& transforms,
        const glm::mat4& view,
        const glm::mat4& projection);

    /*
    * G-Buffer management for deferred rendering
    */
    void resizeGBuffer(int width, int height);

    void geometryPass(const std::vector<Mesh*>& meshes,
        const std::vector<Transform>& transforms,
        const glm::mat4& view,
        const glm::mat4& projection);

    void lightPass(const glm::vec3& cameraPosition, const LightSystem& lightSystem);

    /*
    * Quad rendering (used for post-processing, G-buffer display, etc.)
    */
    void drawScreenQuad();
    void initScreenQuad();

    /*
    * Debugging: Display G-buffer textures (e.g., Position, Normal, Albedo)
    */
    void debugGBuffer(const Shader& debugShader, int debugMode);

private:
    void initOpenGLState();
    void initGBuffer(int width, int height);

    /*
    * Helpers
    */
    void setupMaterial(Shader* shader, const Material* material);
    void setupLight(Shader* shader, const LightSystem& lightSystem, int index);
    void setupGBufferTextures(Shader* shader);

    /*
    * Render Quad
    */
    unsigned int m_quadVAO;

    /*
    * G-buffer resources
    */
    unsigned int m_gBuffer;
    unsigned int m_rboDepth;

    // Store G-buffer textures in a vector
    std::vector<unsigned int> m_gTextures;

    Shader m_gBufferShader;
    Shader m_lightPassShader;

    /*
    * Mesh buffer storage
    */
    struct MeshData {
        unsigned int VAO, VBO, EBO;
        GLsizei indexCount;
        GLsizei vertexCount;
    };

    std::vector<MeshData> m_meshData;
};

#endif // RENDERER_H
