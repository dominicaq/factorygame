#include "engine.h"
#include "scene.h"

#include <string>
#include <vector>
#include <iostream>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// Define globals
InputManager inputManager;
int DEBUG_view_framebuffers = -1;

int main() {
    // Initialize window
    Window window("Factory Game", SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!window.init()) {
        return -1;
    }

    // Scene data
    ECSWorld world;
    LightSystem lightSystem;
    inputManager.init(window.getGLFWwindow());

    // Initialize GameObjectManager
    GameObjectManager gameObjectManager(world);

    // Load the scene data
    Scene::loadScene(world, lightSystem, gameObjectManager);

    // Batched queries from scene
    std::vector<Entity> renderQuery = world.batchedQuery<Mesh, ModelMatrix>();
    std::vector<Entity> modelQuery = world.batchedQuery<ModelMatrix>();

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

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    gameObjectManager.startAll();

    // Game loop
    while (!window.shouldClose()) {
        // Calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;

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

        // Get view matrix from the camera
        glm::mat4 view = world.getResource<Camera>().getViewMatrix();
        // Transform::updateTransforms(world);
        // Transform::updateChildObjects(world);
        Transform::updateModelMatrices(world, modelQuery);

        // Deferred passes
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
