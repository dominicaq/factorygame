#include "system/window.h"

#include "renderer/shader.h"
#include "renderer/renderer.h"
#include "renderer/texture.h"

#include "resources/mesh.h"
#include "resources/meshgen.h"
#include "resources/resource_loader.h"

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

    // Create renderer
    Renderer renderer(SCREEN_WIDTH, SCREEN_HEIGHT);
    window.setRenderer(&renderer);

#pragma region GAME_OBJECT_CUBE

    // Geometry Pass Shader (for writing to G-buffer)
    std::string geometryVertexPath = SHADER_DIR + "deferred/geometry.vs";
    std::string geometryFragPath = SHADER_DIR + "deferred/geometry.fs";
    Shader geometryShader(geometryVertexPath, geometryFragPath);

    // Load texture (optional, depends on your deferred shader setup)
    Texture texture(TEXTURE_DIR + "dice.png");

    // Load mesh and send to renderer
    Mesh* cubeMesh = ResourceLoader::loadMesh(MODEL_DIR + "stanfordBunny.obj");
    if (cubeMesh == nullptr) {
        std::cerr << "ERROR::MESHLOADER::LOADMESH::NULLPTR\n";
        return -1;
    }
    renderer.initMeshBuffers(cubeMesh);

    // Set up the cube's transform
    Transform cubeTransform;
    cubeTransform.position = glm::vec3(2.0f, -0.7f, 0.0f);
    cubeTransform.scale = glm::vec3(7.0f);
    cubeTransform.eulerAngles = glm::vec3(0.0f, 0.0f, 0.0f);

#pragma endregion

#pragma region GAME_CAMERA
    // Set up the camera
    Camera camera;
    camera.position = glm::vec3(0.0f, 0.0f, 5.0f);

    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        float(SCREEN_WIDTH) / float(SCREEN_HEIGHT),
        0.1f,
        100.0f
    );

#pragma endregion

    // Variables for delta time calculation
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // Render loop
    while (!window.shouldClose()) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        window.processInput();

        // Rotate the object over time
        cubeTransform.eulerAngles.y += deltaTime * 25.0f;

        // Get the updated matrices
        glm::mat4 model = cubeTransform.getModelMatrix();
        glm::mat4 view = camera.getViewMatrix();

        // Use the geometry shader and set the uniforms
        geometryShader.use();
        geometryShader.setMat4("u_Model", model);
        geometryShader.setMat4("u_View", view);
        geometryShader.setMat4("u_Projection", projection);

        // Render to G-buffer (this will handle drawing the cube)
        renderer.geometryPass(geometryShader);
        // renderer.draw(cubeMesh, geometryShader);

        // Swap buffers and poll events
        window.swapBuffersAndPollEvents();
    }

    return 0;
}
