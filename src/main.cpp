#include "engine.h"

// Scripts
#include "MoveScript.h"
#include "ViewFramebuffers.h"
#include "FreeCamera.h"

#include <string>
#include <vector>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// Asset paths
static const std::string SHADER_DIR = ASSET_DIR "shaders/";
static const std::string MODEL_DIR = ASSET_DIR "models/";
static const std::string TEXTURE_DIR = ASSET_DIR "textures/";

void loadScene(ECSWorld& world, LightSystem& lightSystem, std::vector<GameObject*>& gameObjects) {
    // ------------------------ Setup Camera ------------------------
    Camera camera;
    camera.position = glm::vec3(4.0f, 0.21f, 4.04f);
    camera.eulerAngles = glm::vec3(-2.38f, 239.0f, 0.0f);
    camera.setNearPlane(0.1f);
    camera.setFarPlane(100.0f);
    camera.setFov(50.0f);
    camera.setAspectRatio(SCREEN_WIDTH, SCREEN_HEIGHT);
    world.insertResource<Camera>(camera);

    // ------------------------ Shader Setup ------------------------

    std::string vertexPath = SHADER_DIR + "default.vs";
    std::string fragmentPath = SHADER_DIR + "default.fs";
    Shader* basicShader = new Shader(vertexPath, fragmentPath);

    // --------------------- Stanford Bunny Model ---------------------
    Entity bunnyEntity = world.createEntity();
    Transform bunnyTransform(glm::vec3(0.0f, -0.5f, 0.0f), glm::vec3(5.0f), glm::vec3(0.0f));
    world.addComponent(bunnyEntity, bunnyTransform);
    world.addComponent(bunnyEntity, ModelMatrix());

    Mesh* bunnyMesh = ResourceLoader::loadMesh(MODEL_DIR + "stanfordBunny.obj");
    if (bunnyMesh != nullptr) {
        Material* bunnyMaterial = new Material(basicShader);
        bunnyMaterial->albedoColor = glm::vec3(1.0f, 0.5f, 0.31f);
        bunnyMaterial->isDeferred = true;

        Texture* bunnyAlbedoMap = new Texture(TEXTURE_DIR + "uv_map.jpg");
        bunnyMaterial->albedoMap = bunnyAlbedoMap;

        bunnyMesh->material = bunnyMaterial;
        world.addComponent(bunnyEntity, bunnyMesh);

        // Create GameObject and attach the MoveScript
        GameObject* bunnyObject = new GameObject(bunnyEntity, &world);
        bunnyObject->addScript<MoveScript>();
        gameObjects.push_back(bunnyObject);
    }

    // --------------------- Dummy Entity (global scripts) ------------------
    Entity dummyEntity = world.createEntity();
    GameObject* dummyObject = new GameObject(dummyEntity, &world);
    dummyObject->addScript<ViewFrameBuffers>();
    dummyObject->addScript<FreeCamera>();
    gameObjects.push_back(dummyObject);

    // --------------------- Diablo Model ---------------------
    Entity diabloEntity = world.createEntity();
    Transform diabloTransform(glm::vec3(2.0f, 0.0f, -1.0f), glm::vec3(2.0f), glm::vec3(0.0f));
    world.addComponent(diabloEntity, diabloTransform);
    world.addComponent(diabloEntity, ModelMatrix());

    Mesh* diabloModel = ResourceLoader::loadMesh(MODEL_DIR + "diablo3_pose.obj");
    if (diabloModel != nullptr) {
        Material* diabloMaterial = new Material(basicShader);
        diabloMaterial->albedoColor = glm::vec3(0.7f, 0.7f, 0.7f);
        diabloMaterial->isDeferred = true;

        Texture* diabloAlbedoMap = new Texture(TEXTURE_DIR + "diablo/diablo3_pose_diffuse.tga");
        diabloMaterial->albedoMap = diabloAlbedoMap;

        Texture* diabloNormalMap = new Texture(TEXTURE_DIR + "diablo/diablo3_pose_nm_tangent.tga");
        diabloMaterial->normalMap = diabloNormalMap;

        diabloModel->material = diabloMaterial;
        world.addComponent(diabloEntity, diabloModel);
    }

    // --------------------- Cube Model (Forward Rendering) ---------------------
    std::string forwardVertexPath = SHADER_DIR + "default.vs";
    std::string forwardFragmentPath = SHADER_DIR + "default.fs";
    Shader* forwardShader = new Shader(forwardVertexPath, forwardFragmentPath);

    Entity cubeEntity = world.createEntity();
    Transform cubeTransform(glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(0.5f), glm::vec3(0.0f));
    world.addComponent(cubeEntity, cubeTransform);
    world.addComponent(cubeEntity, ModelMatrix());

    Mesh* cubeMesh = ResourceLoader::loadMesh(MODEL_DIR + "cube.obj");
    if (cubeMesh != nullptr) {
        Material* cubeMaterial = new Material(forwardShader);
        cubeMaterial->albedoColor = glm::vec3(0.2f, 0.7f, 0.2f);  // Greenish color
        cubeMaterial->isDeferred = false;

        cubeMesh->material = cubeMaterial;
        world.addComponent(cubeEntity, cubeMesh);

        // Create GameObject and attach the MoveScript
        GameObject* cubeObject = new GameObject(cubeEntity, &world);
        cubeObject->addScript<MoveScript>();
        gameObjects.push_back(cubeObject);
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

    // Optionally, output light information
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
    std::vector<GameObject*> gameObjects;
    inputManager.init(window.getGLFWwindow());

    // Load the scene data
    loadScene(world, lightSystem, gameObjects);

    // ------------------------ Debug Setup --------------------------

    // Debug Pass Shader (for visualizing G-buffer)
    std::string debugVertexPath = SHADER_DIR + "deferred/debug_gbuff.vs";
    std::string debugFragmentPath = SHADER_DIR + "deferred/debug_gbuff.fs";
    Shader debugShader(debugVertexPath, debugFragmentPath);
    debugShader.use();
    debugShader.setFloat("u_Near", world.getResource<Camera>().getNearPlane());
    debugShader.setFloat("u_Far",  world.getResource<Camera>().getFarPlane());

    // ---------------------------------------------------------------

    // Hide and capture the cursor for free camera movement
    glfwSetInputMode(window.getGLFWwindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Batched queries from scene
    std::vector<Entity> renderQuery = world.batchedQuery<Mesh, Transform, ModelMatrix>();
    std::vector<Entity> modelQuery = world.batchedQuery<Transform, ModelMatrix>();

    // Create renderer and send mesh data to GPU
    Renderer renderer(SCREEN_WIDTH, SCREEN_HEIGHT, &world.getResource<Camera>());
    window.setRenderer(&renderer);
    for (Entity entity : renderQuery) {
        Mesh* mesh = world.getComponent<Mesh>(entity);
        renderer.initMeshBuffers(mesh);
    }

    // Setup gameobject scripts
    for (auto& gameObject : gameObjects) {
        gameObject->startScripts();
    }

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // Game loop
    while (!window.shouldClose()) {
        // Calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;

        // -------------- Temporary Logic (Camera & Input) --------------

        inputManager.update();

        // TEMPORARY
        if (inputManager.isKeyPressed(GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window.getGLFWwindow(), true);
        }

        // Light movement (temporary, can move into another system later)
        float radius = 5.0f;
        float speed = 1.0f;
        lightSystem.positions[0].x = cos(currentFrame * speed) * radius;
        lightSystem.positions[0].z = sin(currentFrame * speed) * radius;

        // ------------------------ Core Update Logic ------------------------

        // Update all scripts in all gameObjects
        for (auto& gameObject : gameObjects) {
            gameObject->updateScripts(deltaTime);
        }

        // ------------------------ Rendering ------------------------

        // Get view matrix from the camera
        glm::mat4 view = world.getResource<Camera>().getViewMatrix();
        updateModelMatrices(world, modelQuery);

        // Render deferred passes
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

    // ------------------------ Cleanup ------------------------

    // Cleanup allocated GameObjects and their components
    for (auto& gameObject : gameObjects) {
        Mesh* mesh = gameObject->getComponent<Mesh>();
        if (mesh != nullptr) {
            delete mesh->material->albedoMap;
            delete mesh->material->normalMap;
            delete mesh->material;
            delete mesh;
        }
        delete gameObject;
    }

    return 0;
}
