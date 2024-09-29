#ifndef RENDERER_H
#define RENDERER_H

#include "framebuffer.h"
#include "rendergraph.h"
#include "shader.h"

// Components
#include "../components/ecs.h"
#include "../components/transform.h"
#include "../components/mesh.h"
#include "../components/light.h"

#include <glad/glad.h>
#include <memory>
#include <vector>

/*
 * The Renderer class is responsible for handling OpenGL rendering,
 * including G-buffer management for deferred rendering and mesh rendering.
 */
class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();

    /*
     * Render a mesh using its buffers
     */
    void draw(const Mesh* mesh);

    /*
     * Initialize and manage mesh buffers
     */
    void initMeshBuffers(Mesh* mesh, bool isStatic = true);
    void deleteMeshBuffer(const Mesh* mesh);

    /*
     * Quad rendering (used for post-processing, G-buffer display, etc.)
     */
    void drawScreenQuad();
    void initScreenQuad();

    /*
     * Recreate G-buffer with new dimensions
     */
    void resizeGBuffer(int width, int height);

    /*
     * Render Passes
     */
    void geometryPass(ECSWorld& world,
        const std::vector<Entity> entities,
        const glm::mat4& view,
        const glm::mat4& projection);

    void lightPass(const glm::vec3& cameraPosition, const LightSystem& lightSystem);

    void forwardPass(ECSWorld& world,
        const std::vector<Entity> entities,
        const glm::mat4& view,
        const glm::mat4& projection);

    /*
     * Debugging: Display G-buffer textures (e.g., Position, Normal, Albedo)
     */
    void debugGBufferPass(const Shader& debugShader, int debugMode);

private:
    /*
     * Screen dimensions
     */
    int m_width;
    int m_height;

    void setupRenderGraph();

    /*
     * Initialize OpenGL state (depth testing, face culling, etc.)
     */
    void initOpenGLState();

    /*
     * Screen-aligned quad for post process / drawing deferred rendering to screen
     */
    unsigned int m_quadVAO;


    // List of framebuffers (in the future)
    std::unique_ptr<Framebuffer> m_gBuffer;

    /*
     * Shaders for the rendering passes
     */
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
