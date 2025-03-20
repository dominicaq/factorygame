#include "engine.h"
#include "scene/scene.h"
#include "config/settings.h"

// Render passes
#include "renderer/framegraph/geometrypass.h"
#include "renderer/framegraph/lightpass.h"
#include "renderer/framegraph/forwardpass.h"
#include "renderer/framegraph/shadowpass.h"
#include "renderer/framegraph/skyboxpass.h"
#include "renderer/framegraph/framegraph.h"
#include "renderer/framegraph/debugpass.h"

#include "debugging/profiler.h"
#include "debugging/pcinfo.h"
#include "imgui/imgui.h"

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

// Define globals
InputManager inputManager;
DebugContext DEBUG_CTX;

config::GraphicsSettings settings;

int main() {
    DEBUG_CTX.mode = -1;
    DEBUG_CTX.numDepthSlices = 50;

    // Initialize window
    Window window("Factory Game", settings.width, settings.height);
    if (!window.init()) {
        return -1;
    }
    inputManager.init(window.getGLFWwindow());
    printPCInfo();

    // ------------------------ Scene Setup --------------------------
    Scene scene;
    scene.loadScene();

    // ----------------------- Renderer Setup -----------------------

    // Create renderer and send mesh data to GPU
    Camera& camera = scene.getPrimaryCamera();
    Renderer renderer(settings, &camera);
    window.setRenderer(&renderer);

    FrameGraph frameGraph(scene);
    frameGraph.addRenderPass(std::make_unique<ShadowPass>());
    frameGraph.addRenderPass(std::make_unique<GeometryPass>());
    frameGraph.addRenderPass(std::make_unique<LightPass>());
    frameGraph.addRenderPass(std::make_unique<SkyboxPass>());
    frameGraph.addRenderPass(std::make_unique<ForwardPass>());
    frameGraph.addRenderPass(std::make_unique<DebugPass>(&camera));

    // Init all meshses
    auto renderQuery = scene.registry.view<Mesh*, ModelMatrix>();
    renderQuery.each([&](auto entity, Mesh* mesh, ModelMatrix&) {
        renderer.initMeshBuffers(mesh);
    });

    // Init mesh buffers for each vector of model matrices
    auto& meshInstances = scene.getMeshInstances();
    for (const auto& pair : scene.getInstanceMap()) {
        size_t instanceID = pair.first;
        const std::vector<glm::mat4>& matrices = pair.second;

        // Initialize mesh buffers with the instanceID
        renderer.initMeshBuffers(meshInstances[instanceID], false, instanceID);
        // Set up instance attributes with the vector of matrices
        renderer.setupInstanceAttributes(instanceID, matrices);
    }

    frameGraph.setupPasses();

    // -------------------- Start Game -------------------
    GameObjectSystem gameObjectSystem(scene.registry);
    TransformSystem transformSystem(scene.registry);
    LightSystem lightSystem(scene.registry);

    gameObjectSystem.startAll();

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    Profiler profiler;
    // -------------------- Game Loop -------------------
    while (!window.shouldClose()) {
        float currentFrame = (float)glfwGetTime();
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
        gameObjectSystem.updateAll(currentFrame, deltaTime);
        profiler.start("Transform");
        transformSystem.updateTransformComponents();
        profiler.end("Transform");
        lightSystem.updateShadowMatrices();
        profiler.end("Systems");

        // ------------------------ Rendering ------------------------
        profiler.start("Rendering");
        // TODO: currently each pass calculates the view matrix.
        // later update a single resource per frame (the view matrix) to each pass
        frameGraph.executePasses(renderer, scene.registry);
        profiler.end("Rendering");

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

    scene.registry.clear();
    return 0;
}
