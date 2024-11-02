#include "engine.h"
#include "scene.h"

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// Define globals
InputManager inputManager;
int DEBUG_view_framebuffers = -1;

// ------------------------ FPS Counter Setup ------------------------
void fpsCounter(Window& window, float deltaTime) {
    // FPS calculation
    static float fpsTimer = 0.0f;
    static int frames = 0;

    frames++;
    fpsTimer += deltaTime;
    if (fpsTimer >= 1.0f) {
        float fps = frames / fpsTimer;
        std::string title = "Factory Game - FPS: " + std::to_string(fps);
        // Update window title with FPS
        window.setTitle(title);
        frames = 0;
        fpsTimer = 0.0f;
    }
}

int main() {
    // Initialize window
    Window window("Factory Game", SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!window.init()) {
        return -1;
    }
    inputManager.init(window.getGLFWwindow());

    // ------------------------ Scene Setup --------------------------
    Scene scene;
    entt::registry registry;
    loadScene(scene, registry);

     // ------------------------ Renderer Setup --------------------------

    // Create renderer and send mesh data to GPU
    Camera camera = getPrimaryCamera(scene, registry);
    Renderer renderer(SCREEN_WIDTH, SCREEN_HEIGHT, &camera);
    window.setRenderer(&renderer);

    // Init all meshses
    auto renderQuery = registry.view<Mesh*, ModelMatrix>();
    renderQuery.each([&](auto entity, Mesh* mesh, ModelMatrix&) {
        renderer.initMeshBuffers(mesh);
    });

    // ------------------------ Debug Setup --------------------------
    std::string debugVertexPath = SHADER_DIR + "deferred/debug_gbuff.vs";
    std::string debugFragmentPath = SHADER_DIR + "deferred/debug_gbuff.fs";
    Shader debugShader(debugVertexPath, debugFragmentPath);
    debugShader.use();
    debugShader.setFloat("u_Near", camera.getNearPlane());
    debugShader.setFloat("u_Far",  camera.getFarPlane());
    // ---------------------------------------------------------------

    GameObjectSystem gameObjectSystem(registry);
    TransformSystem transformSystem(registry);

    gameObjectSystem.startAll();

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // Game loop
    while (!window.shouldClose()) {
        // Calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;

        fpsCounter(window, deltaTime);

        // -------------- Temporary Logic (Camera & Input) --------------
        inputManager.update();

        // TEMPORARY: Exit on ESC key
        if (inputManager.isKeyPressed(GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window.getGLFWwindow(), true);
        }

        // ------------------------ Core Update Logic ------------------------

        gameObjectSystem.updateAll(deltaTime);

        // ------------------------ Rendering ------------------------

        transformSystem.updateTransformComponents();

        glm::mat4 view = camera.getViewMatrix();
        renderer.geometryPass(registry, view);
        renderer.lightPass(registry);

        // Debug rendering
        if (DEBUG_view_framebuffers >= 0) {
            renderer.debugGBufferPass(debugShader, DEBUG_view_framebuffers);
        }

        // Render forward pass
        renderer.forwardPass(registry, view);

        // Forward pass skybox
        renderer.skyboxPass(view, camera.getProjectionMatrix());

        // Swap buffers and poll events
        window.swapBuffersAndPollEvents();
        lastFrame = currentFrame;
    }

    return 0;
}
