#include "system/window.h"

// Math
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

// Renderer
#include "renderer/shader.h"
#include "renderer/renderer.h"
#include "renderer/texture.h"

// Components
#include "components/mesh.h"
#include "components/transform.h"
#include "components/light.h"

// Resources
#include "resources/meshgen.h"
#include "resources/resource_loader.h"

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

    glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 up = glm::normalize(glm::cross(right, front));

    // Get the current mouse position once on startup
    static bool firstMouse = true;
    static double lastX, lastY;

    if (firstMouse) {
        glfwGetCursorPos(window, &lastX, &lastY);
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
    float yOffset = (lastY - mouseY) * sensitivity;

    lastX = mouseX;
    lastY = mouseY;

    // Apply only if there is movement
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
        camera.eulerAngles.y += xOffset;
        camera.eulerAngles.x += yOffset;

        // Constrain the pitch to avoid gimbal lock
        if (camera.eulerAngles.x > 89.0f) {
            camera.eulerAngles.x = 89.0f;
        }
        if (camera.eulerAngles.x < -89.0f) {
            camera.eulerAngles.x = -89.0f;
        }
    }
}

void loadScene(std::vector<Mesh*>& meshes,
               std::vector<Transform>& transforms,
               Renderer* renderer,
               LightSystem& lightSystem,
               std::vector<Mesh*>& forwardMeshes,
               std::vector<Transform>& forwardTransforms)
{
    // Create shader for both models
    std::string vertexPath = SHADER_DIR + "default.vs";
    std::string fragmentPath = SHADER_DIR + "default.fs";
    Shader* basicShader = new Shader(vertexPath, fragmentPath);

    // --------------------- Stanford Bunny Model ---------------------
    Material* bunnyMaterial = new Material(basicShader);
    bunnyMaterial->albedoColor = glm::vec3(1.0f, 0.5f, 0.31f);

    std::string bunnyTexturePath = TEXTURE_DIR + "uv_map.jpg";
    Texture* bunnyAlbedoMap = new Texture(bunnyTexturePath);
    bunnyMaterial->albedoMap = bunnyAlbedoMap;

    Mesh* bunnyMesh = ResourceLoader::loadMesh(MODEL_DIR + "stanfordBunny.obj");
    if (bunnyMesh != nullptr) {
        bunnyMesh->material = bunnyMaterial;
        meshes.push_back(bunnyMesh);
        transforms.push_back(Transform(glm::vec3(0.0f, -0.5f, 0.0f), glm::vec3(5.0f), glm::vec3(0.0f)));
        renderer->initMeshBuffers(bunnyMesh);
    }

    // --------------------- Diablo Model ---------------------
    Material* diabloMaterial = new Material(basicShader);
    diabloMaterial->albedoColor = glm::vec3(0.7f, 0.7f, 0.7f);

    std::string diabloTexturePath = TEXTURE_DIR + "diablo/diablo3_pose_diffuse.tga";
    Texture* diabloTexture = new Texture(diabloTexturePath);
    diabloMaterial->albedoMap = diabloTexture;

    // Load the normal map for the Diablo model
    std::string diabloNormalMapPath = TEXTURE_DIR + "diablo/diablo3_pose_nm_tangent.tga";
    Texture* diabloNormalMap = new Texture(diabloNormalMapPath);
    diabloMaterial->normalMap = diabloNormalMap;

    Mesh* diabloModel = ResourceLoader::loadMesh(MODEL_DIR + "diablo3_pose.obj");
    if (diabloModel != nullptr) {
        diabloModel->material = diabloMaterial;
        meshes.push_back(diabloModel);
        transforms.push_back(Transform(glm::vec3(2.0f, 0.0f, -1.0f), glm::vec3(2.0f), glm::vec3(0.0f)));
        renderer->initMeshBuffers(diabloModel);
    }

    // --------------------- Cube Model (Forward Rendering) ---------------------
    // Create a shader specifically for forward rendering
    std::string forwardVertexPath = SHADER_DIR + "default.vs";
    std::string forwardFragmentPath = SHADER_DIR + "default.fs";
    Shader* forwardShader = new Shader(forwardVertexPath, forwardFragmentPath);

    // Create a material for the cube
    Material* cubeMaterial = new Material(forwardShader);
    cubeMaterial->albedoColor = glm::vec3(0.2f, 0.7f, 0.2f);  // A greenish color

    Mesh* cubeMesh = ResourceLoader::loadMesh(MODEL_DIR + "cube.obj");
    if (cubeMesh != nullptr) {
        cubeMesh->material = cubeMaterial;
        forwardMeshes.push_back(cubeMesh);
        forwardTransforms.push_back(Transform(glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(0.5f), glm::vec3(0.0f)));
        renderer->initMeshBuffers(cubeMesh);
    }

    // --------------------- Light System ---------------------

    // Light 1: Point Light
    lightSystem.addLight(
        glm::vec3(-20.0f, 0.0f, 0.0f),  // Middle-left side
        glm::vec3(0.2f, 0.2f, 0.2f),    // Grayish White color
        1.0f,                          // Light radius
        3.0f,                           // Light intensity
        false,                          // No shadows
        false                           // Point light
    );

    // Light 2: Point Light
    lightSystem.addLight(
        glm::vec3(-2.0f, 2.5f, 2.0f), // Position for a point light
        glm::vec3(0.0f, 0.0f, 0.5f),  // Green light color
        10.0f,                        // Light radius
        1.0f,                         // Light intensity
        false,                        // No shadows
        false                         // Point light
    );

    // See lights being
    bool seeLightData = false;
    if (!seeLightData) {
        return;
    }

    for (unsigned short i = 0; i < lightSystem.size; ++i) {
        // Access light data
        const glm::vec3& position = lightSystem.positions[i];
        const glm::vec3& direction = lightSystem.directions[i];
        const glm::vec3& color = lightSystem.colors[i];
        float radius = lightSystem.radii[i];
        float intensity = lightSystem.lightIntensities[i];
        bool shadowsEnabled = lightSystem.shadowsEnabled[i];
        const glm::mat4& shadowTransform = lightSystem.shadowTransforms[i];
        unsigned int shadowMap = lightSystem.shadowMaps[i];
        bool isDirectional = lightSystem.directionalFlags[i];

        // Example processing: Print light information
        std::cout << "Light ID: " << i << std::endl;
        std::cout << "Position: " << position.x << ", " << position.y << ", " << position.z << std::endl;
        std::cout << "Direction: " << direction.x << ", " << direction.y << ", " << direction.z << std::endl;
        std::cout << "Color: " << color.x << ", " << color.y << ", " << color.z << std::endl;
        std::cout << "Radius: " << radius << std::endl;
        std::cout << "Intensity: " << intensity << std::endl;
        std::cout << "Shadows Enabled: " << (shadowsEnabled ? "Yes" : "No") << std::endl;
        std::cout << "Directional: " << (isDirectional ? "Yes" : "No") << std::endl;
        std::cout << "---------------------------------" << std::endl;
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

    LightSystem lightSystem;

    // Load deferred pass meshes and store them in a vector
    std::vector<Mesh*> meshes;
    std::vector<Transform> transforms;

    // Load forward pass meshes
    std::vector<Mesh*> forwardMeshes;
    std::vector<Transform> forwardTransforms;

    // Load the scene including deferred and forward pass objects
    loadScene(meshes, transforms, &renderer, lightSystem, forwardMeshes, forwardTransforms);

    // Camera setup
    Camera camera;
    camera.position = glm::vec3(4.0f, 0.21f, 4.04f);
    camera.eulerAngles = glm::vec3(-2.38f, 239.0f, 0.0f);

    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        float(SCREEN_WIDTH) / float(SCREEN_HEIGHT),
        nearPlane,
        farPlane
    );

    // Debug Pass Shader (for visualizing G-buffer)
    std::string debugVertexPath = SHADER_DIR + "deferred/debug_gbuff.vs";
    std::string debugFragmentPath = SHADER_DIR + "deferred/debug_gbuff.fs";
    Shader debugShader(debugVertexPath, debugFragmentPath);
    debugShader.use();
    debugShader.setFloat("u_Near", nearPlane);
    debugShader.setFloat("u_Far", farPlane);

    // Variables for delta time calculation
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // Hide and capture the cursor for free camera movement
    glfwSetInputMode(window.getGLFWwindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Main render loop
    int debugMode = -1;
    while (!window.shouldClose()) {
        // Calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process input
        GLFWwindow* glfwWindow = window.getGLFWwindow();
        processCameraInput(glfwWindow, camera, deltaTime);

        if (glfwGetKey(glfwWindow, GLFW_KEY_1) == GLFW_PRESS) {
            // Turn off debug mode
            debugMode = -1;
        }
        if (glfwGetKey(glfwWindow, GLFW_KEY_2) == GLFW_PRESS) {
            // Position
            debugMode = 0;
        }
        if (glfwGetKey(glfwWindow, GLFW_KEY_3) == GLFW_PRESS) {
            // Normal
            debugMode = 1;
        }
        if (glfwGetKey(glfwWindow, GLFW_KEY_4) == GLFW_PRESS) {
            // Albedo
            debugMode = 2;
        }
        if (glfwGetKey(glfwWindow, GLFW_KEY_5) == GLFW_PRESS) {
            // Depth
            debugMode = 3;
        }
        if (glfwGetKey(glfwWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(glfwWindow, true);
        }

        // Rotate the first light in a circular motion
        float radius = 5.0f;  // Set the radius of the circular motion
        float speed = 1.0f;   // Speed of rotation

        // Calculate the new position using sine and cosine for circular motion
        lightSystem.positions[0].x = cos(currentFrame * speed) * radius;
        lightSystem.positions[0].z = sin(currentFrame * speed) * radius;

        // Get view matrix from the camera
        glm::mat4 view = camera.getViewMatrix();

        // Render deferred pass (geometry and lighting)
        renderer.geometryPass(meshes, transforms, view, projection);
        renderer.lightPass(camera.position, lightSystem);

        if (debugMode >= 0) {
            renderer.debugGBufferPass(debugShader, debugMode);
        } else {
            // Render forward pass (draw the cube)
            renderer.forwardPass(forwardMeshes, forwardTransforms, view, projection);
        }

        // Swap buffers and poll events
        window.swapBuffersAndPollEvents();
    }

    return 0;
}
