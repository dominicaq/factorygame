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

void loadScene(std::vector<Mesh*>& meshes, std::vector<Transform>& transforms, Renderer* renderer) {
    // Create shader for both models
    std::string vertexPath = SHADER_DIR + "default.vs";
    std::string fragmentPath = SHADER_DIR + "default.fs";
    Shader* basicShader = new Shader(vertexPath, fragmentPath);

    // --------------------- Stanford Bunny Model ---------------------

    // Create material for the Stanford Bunny
    Material* bunnyMaterial = new Material(basicShader);
    bunnyMaterial->albedoColor = glm::vec3(1.0f, 0.5f, 0.31f);

    // Load diffuse texture for the Stanford Bunny
    std::string bunnyTexturePath = TEXTURE_DIR + "uv_map.jpg";
    Texture* bunnyAlbedoTexture = new Texture(bunnyTexturePath);
    bunnyMaterial->albedoTexture = bunnyAlbedoTexture;

    // Load Stanford Bunny mesh
    Mesh* bunnyMesh = ResourceLoader::loadMesh(MODEL_DIR + "stanfordBunny.obj");
    if (bunnyMesh != nullptr) {
        bunnyMesh->material = bunnyMaterial;
        meshes.push_back(bunnyMesh);
        transforms.push_back(Transform(glm::vec3(0.0f, -0.5f, 0.0f), glm::vec3(5.0f), glm::vec3(0.0f)));
        renderer->initMeshBuffers(bunnyMesh);
    }

    // --------------------- Atlas Model ---------------------

    // Create material for the Atlas model
    Material* atlasMaterial = new Material(basicShader);
    atlasMaterial->albedoColor = glm::vec3(0.7f, 0.7f, 0.7f);

    // Load diffuse texture for the Atlas model
    std::string atlasTexturePath = TEXTURE_DIR + "uv_map.jpg";
    Texture* atlasAlbedoTexture = new Texture(atlasTexturePath);
    atlasMaterial->albedoTexture = atlasAlbedoTexture;

    // Load Atlas mesh
    Mesh* atlasMesh = ResourceLoader::loadMesh(MODEL_DIR + "atlas.obj");
    if (atlasMesh != nullptr) {
        atlasMesh->material = atlasMaterial;
        meshes.push_back(atlasMesh);
        transforms.push_back(Transform(glm::vec3(2.0f, 0.0f, -1.0f), glm::vec3(0.1f), glm::vec3(0.0f)));
        renderer->initMeshBuffers(atlasMesh);
    }
}

int main() {
    // Initialize window
    Window window("Factory Game", SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!window.init()) {
        return -1;
    }

    // Create renderer
    Renderer renderer(SCREEN_WIDTH, SCREEN_HEIGHT);
    window.setRenderer(&renderer);

    // Load meshes and store them in a vector
    std::vector<Mesh*> meshes;
    std::vector<Transform> transforms;
    loadScene(meshes, transforms, &renderer);

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
    int debugMode = 0;
    while (!window.shouldClose()) {
        // Calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process input
        GLFWwindow* glfwWindow = window.getGLFWwindow();
        if (glfwGetKey(glfwWindow, GLFW_KEY_1) == GLFW_PRESS) {
            debugMode = 0;
        }
        if (glfwGetKey(glfwWindow, GLFW_KEY_2) == GLFW_PRESS) {
            debugMode = 1;
        }
        if (glfwGetKey(glfwWindow, GLFW_KEY_3) == GLFW_PRESS) {
            debugMode = 2;
        }
        if (glfwGetKey(glfwWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(glfwWindow, true);
        }

        // Rotate all meshes in the Y axis based on delta time
        for (Transform& transform : transforms) {
            transform.eulerAngles.y += deltaTime * 10.0f;
        }

        // Get view matrix from the camera
        glm::mat4 view = camera.getViewMatrix();

        // Render passes
        renderer.geometryPass(meshes, transforms, view, projection);

        // TODO: Lighting pass

        // Final pass (draw to screen)
        renderer.debugGBuffer(debugShader, debugMode);

        window.swapBuffersAndPollEvents();
    }

    return 0;
}
