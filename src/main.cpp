#include "engine.h"

// Scripts
#include "MoveScript.h"

#include <string>
#include <vector>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// Asset paths
static const std::string SHADER_DIR = ASSET_DIR "shaders/";
static const std::string MODEL_DIR = ASSET_DIR "models/";
static const std::string TEXTURE_DIR = ASSET_DIR "textures/";

void freeCamera(GLFWwindow* window, ECSWorld& world, float deltaTime) {
    // Camera speed
    const float cameraSpeed = 2.5f * deltaTime;
    const float sensitivity = 0.1f;

    Camera& camera = world.getResource<Camera>();

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
        bunnyObject->addScript<MoveScript>();  // Attach MoveScript
        gameObjects.push_back(bunnyObject);
    }

    // --------------------- Dummy Entity ----------------------
    Entity dummyEntity = world.createEntity();

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
        // cubeObject->addScript<MoveScript>();
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

void glfwControls(GLFWwindow* glfwWindow, int& debugMode) {
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
}

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

    // Load the scene data
    loadScene(world, lightSystem, gameObjects);

    // Setup gameobject scripts
    for (auto& gameObject : gameObjects) {
        gameObject->startScripts();
    }

    // ------------------------ Debug Setup --------------------------

    // Debug Pass Shader (for visualizing G-buffer)
    std::string debugVertexPath = SHADER_DIR + "deferred/debug_gbuff.vs";
    std::string debugFragmentPath = SHADER_DIR + "deferred/debug_gbuff.fs";
    Shader debugShader(debugVertexPath, debugFragmentPath);
    debugShader.use();
    debugShader.setFloat("u_Near", world.getResource<Camera>().getNearPlane());
    debugShader.setFloat("u_Far",  world.getResource<Camera>().getFarPlane());

    // Control which gbuffer texture is shown
    int debugMode = -1;

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

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // Game loop
    while (!window.shouldClose()) {
        // Calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;

        // -------------- Temporary Logic (Camera & Input) --------------

        // Process input for camera movement
        GLFWwindow* glfwWindow = window.getGLFWwindow();
        freeCamera(glfwWindow, world, deltaTime);
        glfwControls(glfwWindow, debugMode);

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
        if (debugMode >= 0) {
            renderer.debugGBufferPass(debugShader, debugMode);
        }

        // Render forward pass
        renderer.forwardPass(world, renderQuery, view);

        // Swap buffers and poll events
        window.swapBuffersAndPollEvents();
        lastFrame = currentFrame;
    }

    // ------------------------ Cleanup ------------------------

    // Cleanup allocated GameObjects
    for (auto& gameObject : gameObjects) {
        delete gameObject;
    }

    return 0;
}
