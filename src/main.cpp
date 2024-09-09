#include "system/window.h"
#include "renderer/shader.h"
#include "renderer/renderer.h"
#include "resources/mesh.h"
#include "resources/mesh_loader.h"

#include <string>

/*
* Asset paths
*/
static const std::string SHADER_DIR = ASSET_DIR "shaders/";
static const std::string MODEL_DIR = ASSET_DIR "models/";
static const std::string TEXTURE_DIR = ASSET_DIR "textures/";

int main() {
    Window window("Factory Game", 800, 600);
    if (!window.init()) {
        return -1;
    }

    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    // Set up the viewport after window initialization
    glViewport(0, 0, 800, 600);

    // Hard coded asset directories for now
    std::string vertexPath = SHADER_DIR + "default_vertex.glsl";
    std::string fragPath = SHADER_DIR + "default_fragment.glsl";
    Shader shader(vertexPath, fragPath);

    std::string cubePath = MODEL_DIR + "cube.obj";
    Mesh* mesh = MeshLoader::loadMesh(cubePath, MeshLoader::FileType::OBJ);
    if (mesh == nullptr) {
        std::cerr << "ERROR::MESHLOADER::LOADMESH::NULLPTR\n";
        return -1;
    }

    // Create the renderer
    Renderer renderer;
    renderer.init();

    // Create transformation matrices
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(0.5f));  // Scale down the cube

    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 5.0f),  // Move camera farther back
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        800.0f / 600.0f,
        0.1f,
        100.0f
    );

    glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
    glm::vec3 objectColor(1.0f, 0.5f, 0.31f);

    // Variables for delta time calculation
    float deltaTime = 0.0f;  // Time between current frame and last frame
    float lastFrame = 0.0f;  // Time of the last frame

    // Render loop
    while (!window.shouldClose()) {
        // Calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        window.processInput();

        // Clear screen and depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update model matrix for rotation (scaled by delta time)
        model = glm::rotate(model, deltaTime * glm::radians(50.0f), glm::vec3(1.0f, 1.0f, 0.0f));

        // Calculate MVP matrix
        glm::mat4 mvp = projection * view * model;

        // Use shader and set uniforms
        shader.use();
        shader.setMat4("u_MVP", mvp);

        // Render mesh
        renderer.render(mesh, shader);

        window.swapBuffersAndPollEvents();
    }

    return 0;
}
