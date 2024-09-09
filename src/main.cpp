#include "system/window.h"
#include "renderer/shader.h"
#include "renderer/renderer.h"
#include "resources/mesh.h"

#include <string>

/*
* Asset path(s)
*/
static const std::string SHADER_DIR = ASSET_DIR "shaders/";

int main() {
    Window window("Factory Game", 800, 600);
    if (!window.init()) {
        return -1;
    }

    // Set up the viewport after window initialization
    glViewport(0, 0, 800, 600);

    // Hard coded asset directories for now
    std::string vertexPath = SHADER_DIR + "default_vertex.glsl";
    std::string fragPath = SHADER_DIR + "default_fragment.glsl";
    Shader shader(vertexPath, fragPath);

    Mesh mesh;
    mesh.vertices = {
        glm::vec3( 0.0f,  0.5f, 0.0f),  // Top vertex
        glm::vec3(-0.5f, -0.5f, 0.0f),  // Bottom-left vertex
        glm::vec3( 0.5f, -0.5f, 0.0f)   // Bottom-right vertex
    };
    mesh.indices = { 0, 1, 2 };

    // Create the renderer
    Renderer renderer;
    renderer.init();

    // Render loop
    while (!window.shouldClose()) {
        window.processInput();

        // Set the background to black and clear the screen
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render the mesh with the shader
        renderer.render(mesh, shader);

        window.swapBuffersAndPollEvents();
    }

    return 0;
}
