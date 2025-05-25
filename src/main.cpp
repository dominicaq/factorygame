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

// Define globals
InputManager inputManager;
DebugContext DEBUG_CTX;

config::GraphicsSettings settings;

int main() {
    DEBUG_CTX.mode = -1;
    DEBUG_CTX.numDepthSlices = 50;

    // Initialize window
    Window window("Factory Game", settings.display.width, settings.display.height);
    if (!window.init()) {
        return -1;
    }
    inputManager.init(window.getGLFWwindow());
    // Init editor specific UI
    Profiler profiler;
    // Editor editor(settings.display.width, settings.display.height);
    std::cout << "[Info] Success. Running engine setup...\n";

    // ------------------------ Scene Setup --------------------------
    Scene scene;
    scene.loadScene();

    // ----------------------- FrameGraph Setup -----------------------

    // Create renderer and send mesh data to GPU
    Camera& camera = scene.getPrimaryCamera();
    Renderer renderer(settings, &camera);
    window.setRenderer(&renderer);
    // editor.setRenderer(&renderer);

    // Create the frame graph with scene
    FrameGraph frameGraph(scene);
    frameGraph.addRenderPass(std::make_unique<ShadowPass>());
    frameGraph.addRenderPass(std::make_unique<GeometryPass>());

    auto skyboxPass = std::make_unique<SkyboxPass>();
    skyboxPass->setSkyBox(scene.getSkyBox());
    frameGraph.addRenderPass(std::move(skyboxPass));

    // frameGraph.addRenderPass(std::make_unique<ForwardPass>());

    auto lightPass = std::make_unique<LightPass>();
    lightPass->setSkyBox(scene.getSkyBox());
    frameGraph.addRenderPass(std::move(lightPass));

    // Post processing passes. Debug pass is like a post process.
    frameGraph.addRenderPass(std::make_unique<DebugPass>(&camera));

    // ----------------------- Renderer Setup -----------------------
    MaterialManager& matManager = MaterialManager::getInstance();
    matManager.initialize(500);

    // Init all meshes
    for (auto& meshDef : scene.meshEntityPairs) {
        uint32_t materialIndex = matManager.getMaterialIndex(*meshDef.materialDef);

        Mesh& mesh = renderer.initMeshBuffers(meshDef.rawMeshData);
        mesh.materialIndex = materialIndex;
        scene.registry.emplace<Mesh>(meshDef.entity, mesh);
    }

    // Instanced mesh groups
    for (auto& instanceGroup : scene.instancedMeshGroups) {
        // Get or create material data for instance group
        uint32_t materialIndex = matManager.getMaterialIndex(*instanceGroup.materialDef);
        Mesh& instancedMesh = renderer.initMeshBuffers(instanceGroup.meshData);

        instancedMesh.materialIndex = materialIndex;
        instanceGroup.initializedMesh = &instancedMesh;

        // Add the same Mesh component (with material index) to all entities in the group
        for (entt::entity entity : instanceGroup.entities) {
            scene.registry.emplace<Mesh>(entity, instancedMesh);
        }
    }
    matManager.updateMaterialBuffer();

    // -------------------- Start Game -------------------
    frameGraph.setupPasses();
    GameObjectSystem gameObjectSystem(scene.registry);
    gameObjectSystem.setOnDirtyInstanceCallback([&scene]() {
        scene.flagDirtyInstanceCount();
    });
    TransformSystem transformSystem(scene.registry);
    LightSystem lightSystem(settings, scene.getPrimaryCamera(), scene.registry);

    std::cout << "[Info] Success. Starting game...\n";
    printPCInfo();
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

        // // ------------------ ImGui Rendering ------------------
        window.beginImGuiFrame();
        profiler.record(1.0f / deltaTime);
        profiler.display();
        // editor.drawEditorLayout(scene, renderer);
        window.endImGuiFrame();

        // Swap buffers and poll events
        window.swapBuffersAndPollEvents();
        lastFrame = currentFrame;
    }

    scene.registry.clear();
    return 0;
}
