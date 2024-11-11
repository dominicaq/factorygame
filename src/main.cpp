#include "engine.h"

// Render passes
#include "renderer/rendergraph/geometrypass.h"
#include "renderer/rendergraph/lightpass.h"
#include "renderer/rendergraph/forwardpass.h"
#include "renderer/rendergraph/rendergraph.h"

#include "debugging/profiler.h"
#include "imgui/imgui.h"

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

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
    inputManager.init(window.getGLFWwindow());

    // ------------------------ Scene Setup --------------------------
    Scene scene;
    scene.loadScene();

    // ----------------------- Renderer Setup -----------------------

    // Create renderer and send mesh data to GPU
    Camera& camera = scene.getPrimaryCamera();
    Renderer renderer(SCREEN_WIDTH, SCREEN_HEIGHT, &camera);
    window.setRenderer(&renderer);

    FrameGraph frameGraph;
    frameGraph.addRenderPass(std::make_unique<GeometryPass>());
    frameGraph.addRenderPass(std::make_unique<LightPass>());
    frameGraph.addRenderPass(std::make_unique<ForwardPass>());

    // Init all meshses
    auto renderQuery = scene.registry.view<Mesh*, ModelMatrix>();
    renderQuery.each([&](auto entity, Mesh* mesh, ModelMatrix&) {
        renderer.initMeshBuffers(mesh);
    });

    frameGraph.setupPasses();
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

    Profiler profiler;

    // Game loop
    while (!window.shouldClose()) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;

        // -------------- Input Management -----------
        profiler.start("Frame");
        inputManager.update();

        // TEMPORARY: Exit on ESC key
        if (inputManager.isKeyPressed(GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window.getGLFWwindow(), true);
        }

        // -------------- System updates ------------

        profiler.start("Systems");
        gameObjectSystem.updateAll(deltaTime);
        transformSystem.updateTransformComponents();
        profiler.end("Systems");

        // ------------------------ Rendering ------------------------
        profiler.start("Rendering");
        // TODO: currently each pass calculates the view matrix.
        // later update a single resource per frame (the view matrix) to each pass
        frameGraph.executePasses(renderer, scene.registry);
        renderer.skyboxPass(camera.getViewMatrix(), camera.getProjectionMatrix());
        profiler.end("Rendering");

        // DEBUG MODE
        if (DEBUG_view_framebuffers >= 0) {
            renderer.debugGBufferPass(debugShader, DEBUG_view_framebuffers);
        }

        profiler.end("Frame");

        // ------------------ ImGui Rendering ------------------
        window.beginImGuiFrame();
        profiler.record(1.0f / deltaTime);
        profiler.display();
        window.endImGuiFrame();

        // Swap buffers and poll events
        window.swapBuffersAndPollEvents();
        lastFrame = currentFrame;
    }

    return 0;
}
