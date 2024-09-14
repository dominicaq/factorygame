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

// TEMPORARY
#include "transform.h"

#include <string>
#include <vector>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

/*
* Asset paths
*/
static const std::string SHADER_DIR = ASSET_DIR "shaders/";
static const std::string MODEL_DIR = ASSET_DIR "models/";
static const std::string TEXTURE_DIR = ASSET_DIR "textures/";

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
    // Initialize window
    Window window("Factory Game", SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!window.init()) {
        return -1;
    }

    // Create renderer
    Renderer renderer(SCREEN_WIDTH, SCREEN_HEIGHT);
    window.setRenderer(&renderer);

    // Geometry Pass Shader (for writing to G-buffer)
    std::string gBufferVertexPath = SHADER_DIR + "deferred/gbuff.vs";
    std::string gBufferFragmentPath = SHADER_DIR + "deferred/gbuff.fs";
    Shader gBufferShader(gBufferVertexPath, gBufferFragmentPath);

    // Load meshes and store them in a vector
    std::vector<Mesh*> meshes;
    std::vector<Transform> transforms;

    // Load first mesh (Stanford Bunny)
    Mesh* bunnyMesh = ResourceLoader::loadMesh(MODEL_DIR + "stanfordBunny.obj");
    if (bunnyMesh != nullptr) {
        meshes.push_back(bunnyMesh);
        transforms.push_back(Transform(glm::vec3(0.0f, -0.5f, 0.0f), glm::vec3(5.0f), glm::vec3(0.0f)));
        renderer.initMeshBuffers(bunnyMesh);
    }

    // Load second mesh (Another Model)
    Mesh* anotherMesh = ResourceLoader::loadMesh(MODEL_DIR + "atlas.obj");
    if (anotherMesh != nullptr) {
        meshes.push_back(anotherMesh);
        transforms.push_back(Transform(glm::vec3(2.0f, 0.0f, -1.0f), glm::vec3(0.1f), glm::vec3(0.0f)));
        renderer.initMeshBuffers(anotherMesh);
    }

    // Camera setup
    Camera camera;
    camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        float(SCREEN_WIDTH) / float(SCREEN_HEIGHT),
        0.1f,
        100.0f
    );

    // Debug Pass Shader (for visualizing G-buffer)
    std::string debugVertexPath = SHADER_DIR + "deferred/quad.vs";
    std::string debugFragmentPath = SHADER_DIR + "deferred/quad.fs";
    Shader debugShader(debugVertexPath, debugFragmentPath);

    // Variables for delta time calculation
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // Main render loop
    while (!window.shouldClose()) {
        // Calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process input
        window.processInput();

        // Rotate all meshes in the Y axis based on delta time
        for (auto& transform : transforms) {
            transform.eulerAngles.y += deltaTime * 10.0f;
        }

        // Get view matrix from the camera
        glm::mat4 view = camera.getViewMatrix();

        // Geometry pass (write to G-buffer)
        gBufferShader.use();
        gBufferShader.setMat4("u_View", view);
        gBufferShader.setMat4("u_Projection", projection);
        renderer.geometryPass(gBufferShader, meshes, transforms);

        // Debug pass (render the G-buffer)
        renderer.debugGBuffer(debugShader, 1);

        // Swap buffers and poll events
        window.swapBuffersAndPollEvents();
    }

    return 0;
}
