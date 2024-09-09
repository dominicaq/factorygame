#include "system/window.h"
#include "renderer/shader.h"
#include "renderer/renderer.h"
#include "resources/mesh.h"
#include "resources/mesh_loader.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include <string>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

/*
* Asset paths
*/
static const std::string SHADER_DIR = ASSET_DIR "shaders/";
static const std::string MODEL_DIR = ASSET_DIR "models/";
static const std::string TEXTURE_DIR = ASSET_DIR "textures/";

struct Transform {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    glm::vec3 eulerAngles = glm::vec3(0.0f);

    glm::mat4 getModelMatrix() const {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(eulerAngles.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(eulerAngles.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(eulerAngles.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, scale);
        return model;
    }
};

struct Camera {
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 5.0f);
    glm::vec3 eulerAngles = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::mat4 getViewMatrix() const {
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::rotate(view, glm::radians(eulerAngles.x), glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, glm::radians(eulerAngles.y), glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::rotate(view, glm::radians(eulerAngles.z), glm::vec3(0.0f, 0.0f, 1.0f));
        view = glm::translate(view, -position);
        return view;
    }
};

int main() {
    Window window("Factory Game", SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!window.init()) {
        return -1;
    }

    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    // Set up the viewport after window initialization
    glViewport(0, 0, 800, 600);

    // Load shader
    std::string vertexPath = SHADER_DIR + "default_vertex.glsl";
    std::string fragPath = SHADER_DIR + "default_fragment.glsl";
    Shader shader(vertexPath, fragPath);

    // Load mesh
    std::string cubePath = MODEL_DIR + "cube.obj";
    Mesh* cubeMesh = MeshLoader::loadMesh(cubePath, MeshLoader::FileType::OBJ);
    if (cubeMesh == nullptr) {
        std::cerr << "ERROR::MESHLOADER::LOADMESH::NULLPTR\n";
        return -1;
    }

    // Create the renderer
    Renderer renderer;
    renderer.init();
    renderer.setupMesh(cubeMesh);

    // Set up the cube's transform
    Transform cubeTransform;
    cubeTransform.position = glm::vec3(0.0f, 0.0f, 0.0f);
    cubeTransform.scale = glm::vec3(0.5f);  // Scale down the cube
    cubeTransform.eulerAngles = glm::vec3(0.0f, 0.0f, 0.0f);  // No rotation initially

    // Set up the camera
    Camera camera;
    camera.position = glm::vec3(0.0f, 0.0f, 5.0f);

    // Set up projection matrix
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        float(SCREEN_WIDTH) / float(SCREEN_HEIGHT),
        0.1f,
        100.0f
    );

    // Variables for delta time calculation
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // Render loop
    while (!window.shouldClose()) {
        // Calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        window.processInput();

        // Clear screen and depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Rotate the cube over time
        cubeTransform.eulerAngles.y += deltaTime * 50.0f;  // Rotate around y-axis

        // Get the updated matrices
        glm::mat4 model = cubeTransform.getModelMatrix();
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 mvp = projection * view * model;

        // Use shader and set uniforms
        shader.use();
        shader.setMat4("u_MVP", mvp);

        // Render the cube mesh
        renderer.render(cubeMesh, shader);

        window.swapBuffersAndPollEvents();
    }

    return 0;
}
