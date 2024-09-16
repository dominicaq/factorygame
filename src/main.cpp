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
        glm::vec3 front;
        front.x = cos(glm::radians(eulerAngles.y)) * cos(glm::radians(eulerAngles.x));
        front.y = sin(glm::radians(eulerAngles.x)); // Pitch controls vertical rotation
        front.z = sin(glm::radians(eulerAngles.y)) * cos(glm::radians(eulerAngles.x));
        front = glm::normalize(front);

        // The world up vector for an FPS camera is always (0, 1, 0)
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        // The right vector is always perpendicular to the front and up vector
        glm::vec3 right = glm::normalize(glm::cross(front, up));

        // Recompute the up vector (it could be tilted slightly)
        up = glm::normalize(glm::cross(right, front));

        return glm::lookAt(position, position + front, up);
    }
};

void processCameraInput(GLFWwindow* window, Camera& camera, float deltaTime) {
    // Camera speed
    const float cameraSpeed = 2.5f * deltaTime;
    const float sensitivity = 0.1f;

    // Calculate camera front vector based on Euler angles
    glm::vec3 front;
    front.x = cos(glm::radians(camera.eulerAngles.y)) * cos(glm::radians(camera.eulerAngles.x));
    front.y = sin(glm::radians(camera.eulerAngles.x));
    front.z = sin(glm::radians(camera.eulerAngles.y)) * cos(glm::radians(camera.eulerAngles.x));
    front = glm::normalize(front);

    glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f))); // Right vector
    glm::vec3 up = glm::normalize(glm::cross(right, front));                           // Up vector

    // Get the current mouse position once on startup
    static bool firstMouse = true;
    static double lastX, lastY;

    if (firstMouse) {
        glfwGetCursorPos(window, &lastX, &lastY);  // Initialize to the current mouse position
        firstMouse = false;
    }

    // Keyboard input for movement (WASD)
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.position += cameraSpeed * front;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.position -= cameraSpeed * front;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.position -= right * cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.position += right * cameraSpeed;
    }

    // Mouse input for look rotation
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    float xOffset = (mouseX - lastX) * sensitivity;
    float yOffset = (lastY - mouseY) * sensitivity;  // Inverted for typical FPS controls

    lastX = mouseX;
    lastY = mouseY;

    // Apply only if there is movement
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
        camera.eulerAngles.y += xOffset;  // Yaw (left/right rotation)
        camera.eulerAngles.x += yOffset;  // Pitch (up/down rotation)

        // Constrain the pitch to avoid gimbal lock
        if (camera.eulerAngles.x > 89.0f) {
            camera.eulerAngles.x = 89.0f;
        }
        if (camera.eulerAngles.x < -89.0f) {
            camera.eulerAngles.x = -89.0f;
        }
    }
}

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
    // Set camera position at (0,0,0)
    camera.position = glm::vec3(4.0f, 0.21f, 4.04f);
    camera.eulerAngles = glm::vec3(-2.38f, 239.0f, 0.0f);

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

    // Hide and capture the cursor for free camera movement
    glfwSetInputMode(window.getGLFWwindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Main render loop
    int debugMode = 0;
    while (!window.shouldClose()) {
        // Calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process input
        GLFWwindow* glfwWindow = window.getGLFWwindow();
        processCameraInput(glfwWindow, camera, deltaTime);

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
