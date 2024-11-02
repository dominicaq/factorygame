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
    scene.loadScene();

    // ----------------------- Renderer Setup -----------------------

    // Create renderer and send mesh data to GPU
    Camera& camera = scene.getPrimaryCamera();
    Renderer renderer(SCREEN_WIDTH, SCREEN_HEIGHT, &camera);
    window.setRenderer(&renderer);

    // Init all meshses
    auto renderQuery = scene.registry.view<Mesh*, ModelMatrix>();
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

    GameObjectSystem gameObjectSystem(scene.registry);
    TransformSystem transformSystem(scene.registry);

    gameObjectSystem.startAll();

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // Game loop
    while (!window.shouldClose()) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        fpsCounter(window, deltaTime);

        // -------------- Input Management -----------
        inputManager.update();

        // TEMPORARY: Exit on ESC key
        if (inputManager.isKeyPressed(GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window.getGLFWwindow(), true);
        }

        // -------------- System updates ------------

        gameObjectSystem.updateAll(deltaTime);
        transformSystem.updateTransformComponents();

        // ------------------------ Rendering ------------------------
        glm::mat4 view = camera.getViewMatrix();
        renderer.geometryPass(scene.registry, view);
        renderer.lightPass(scene.registry);
        renderer.forwardPass(scene.registry, view);
        renderer.skyboxPass(view, camera.getProjectionMatrix());

        // DEBUG MODE
        if (DEBUG_view_framebuffers >= 0) {
            renderer.debugGBufferPass(debugShader, DEBUG_view_framebuffers);
        }

        // Swap buffers and poll events
        window.swapBuffersAndPollEvents();
        lastFrame = currentFrame;
    }

    return 0;
}
