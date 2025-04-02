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

#include "editor/editor.h"
#include "editor/profiler.h"
#include "editor/pcinfo.h"

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

    // Init editor specific UI
    Profiler profiler;
    Editor editor(settings.width, settings.height);
    std::cout << "Success. Running engine setup...\n";

    // ------------------------ Scene Setup --------------------------
    Scene scene;
    scene.loadScene();

    // ----------------------- FrameGraph Setup -----------------------

    // Create renderer and send mesh data to GPU
    Camera& camera = scene.getPrimaryCamera();
    Renderer renderer(settings, &camera);
    window.setRenderer(&renderer);
    editor.setRenderer(&renderer);

    // Create the frame graph with scene
    FrameGraph frameGraph(scene);
    frameGraph.addRenderPass(std::make_unique<ShadowPass>());
    frameGraph.addRenderPass(std::make_unique<GeometryPass>());

    auto skyboxPass = std::make_unique<SkyboxPass>();
    skyboxPass->setSkyBox(scene.getSkyBox());
    frameGraph.addRenderPass(std::move(skyboxPass));

    frameGraph.addRenderPass(std::make_unique<ForwardPass>());

    auto lightPass = std::make_unique<LightPass>();
    lightPass->setSkyBox(scene.getSkyBox());
    frameGraph.addRenderPass(std::move(lightPass));

    // Post processing passes. Debug pass is like a post process.
    frameGraph.addRenderPass(std::make_unique<DebugPass>(&camera));

    // ----------------------- Renderer Setup -----------------------
    // Init all meshses
    scene.registry.view<Mesh*, ModelMatrix>().each([&](entt::entity entity, Mesh* mesh, const ModelMatrix& modelMatrix) {
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

    // -------------------- Start Game -------------------
    frameGraph.setupPasses();
    GameObjectSystem gameObjectSystem(scene.registry);
    TransformSystem transformSystem(scene.registry);
    LightSystem lightSystem(scene.registry);

    // printPCInfo();
    std::cout << "Success. Starting game...\n";
    gameObjectSystem.startAll();

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

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
        profiler.start("Shadow");
        lightSystem.updateShadowMatrices();
        profiler.end("Shadow");
        profiler.end("Systems");

        // ------------------------ Rendering ------------------------
        profiler.start("Rendering");
        // TODO: currently each pass calculates the view matrix.
        // later update a single resource per frame (the view matrix) to each pass
        // TODO: maybe also do this for skybox?
        frameGraph.executePasses(renderer, scene.registry);
        profiler.end("Rendering");
        profiler.end("Frame");

        // ------------------ ImGui Rendering ------------------
        window.beginImGuiFrame();
        profiler.record(1.0f / deltaTime);
        profiler.display();
        editor.drawEditorLayout(scene, renderer);
        window.endImGuiFrame();

        // Swap buffers and poll events
        window.swapBuffersAndPollEvents();
        lastFrame = currentFrame;
    }

    scene.registry.clear();
    return 0;
}
