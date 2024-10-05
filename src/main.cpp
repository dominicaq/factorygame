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
    ECSWorld world;
    // TODO: thinking about turning gameobject into a component, not sure
    GameObjectManager gameObjectManager(world);

    // TODO: deprecate lightsystem
    LightSystem lightSystem;

    // Load the scene data
    Scene::loadScene(world, lightSystem, gameObjectManager);

     // ------------------------ Systems Setup --------------------------
    TransformSystem transformSystem(world);

     // ------------------------ Renderer Setup --------------------------
    std::vector<Entity> renderQuery = world.batchedQuery<Mesh, ModelMatrix>();

    // Create renderer and send mesh data to GPU
    Renderer renderer(SCREEN_WIDTH, SCREEN_HEIGHT, &world.getResource<Camera>());
    window.setRenderer(&renderer);
    for (Entity entity : renderQuery) {
        Mesh* mesh = world.getComponent<Mesh>(entity);
        if (mesh) {
            renderer.initMeshBuffers(mesh);
        }
    }

    // ------------------------ Debug Setup --------------------------
    // Debug Pass Shader (for visualizing G-buffer)
    std::string debugVertexPath = SHADER_DIR + "deferred/debug_gbuff.vs";
    std::string debugFragmentPath = SHADER_DIR + "deferred/debug_gbuff.fs";
    Shader debugShader(debugVertexPath, debugFragmentPath);
    debugShader.use();
    debugShader.setFloat("u_Near", world.getResource<Camera>().getNearPlane());
    debugShader.setFloat("u_Far",  world.getResource<Camera>().getFarPlane());
    // ---------------------------------------------------------------

    gameObjectManager.startAll();

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

        // Light movement (temporary, can move into another system later)
        float radius = 5.0f;
        float speed = 1.0f;
        lightSystem.positions[0].x = cos(currentFrame * speed) * radius;
        lightSystem.positions[0].z = sin(currentFrame * speed) * radius;

        // ------------------------ Core Update Logic ------------------------

        gameObjectManager.updateAll(deltaTime);

        // ------------------------ Rendering ------------------------

        transformSystem.updateTransformComponents();

        // Deferred passes
        glm::mat4 view = world.getResource<Camera>().getViewMatrix();
        renderer.geometryPass(world, renderQuery, view);
        renderer.lightPass(world, lightSystem);

        // Debug rendering
        if (DEBUG_view_framebuffers >= 0) {
            renderer.debugGBufferPass(debugShader, DEBUG_view_framebuffers);
        }

        // Render forward pass
        renderer.forwardPass(world, renderQuery, view);

        // Forward pass skybox
        renderer.skyboxPass(view, world.getResource<Camera>().getProjectionMatrix());

        // Swap buffers and poll events
        window.swapBuffersAndPollEvents();
        lastFrame = currentFrame;
    }

    return 0;
}
