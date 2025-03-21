#include "scene.h"
#include <random>

// User scripts
#include "CircularRotation.h"
#include "FreeCamera.h"
#include "MoveScript.h"
#include "BouncingMotion.h"
#include "ViewFramebuffers.h"

/*
* Scene Management
*/

// TODO: In the future, this will be user generated through UI, not code.
void Scene::loadScene() {
    // ------------------------ Wireframe Setup ------------------------
    Shader* wireframeShader = new Shader(SHADER_DIR + "debug/wireframe.vs", SHADER_DIR + "debug/wireframe.fs");
    m_wireframeMaterial = new Material(wireframeShader);
    m_wireframeMaterial->albedoColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

    // ------------------------ Setup Camera ------------------------
    entt::entity cameraEntity = registry.create();
    SceneData save_cameraData;
    save_cameraData.name = "Main Camera";
    save_cameraData.position = glm::vec3(4.0f, 0.21f, 4.04f);
    save_cameraData.eulerAngles = glm::vec3(-2.38f, 239.0f, 0.0f);
    GameObject* cameraObject = SceneUtils::addGameObjectComponent(registry, cameraEntity, save_cameraData);
    cameraObject->addScript<FreeCamera>();

    Camera cameraComponent(cameraEntity, registry);
    registry.emplace<Camera>(cameraEntity, cameraComponent);
    setPrimaryCamera(cameraEntity);

    // ------------------------ Shader Setup ------------------------
    std::string vertexPath = SHADER_DIR + "default.vs";
    std::string fragmentPath = SHADER_DIR + "default.fs";
    Shader* basicShader = new Shader(vertexPath, fragmentPath);

    // --------------------- Beetle Car ---------------------
    const int numRows = 5;  // Adjust for more rows
    const float spacing = 10.0f;  // Distance between rows
    const float startZ = -((numRows - 1) * spacing) / 2.0f;  // Center the rows

    // Create Bunny
    entt::entity bunnyEntity = registry.create();
    SceneData save_bunnyData;
    save_bunnyData.name = "Bunny";
    save_bunnyData.position = glm::vec3(6.0f, -0.5f, startZ); // Set to first buggy position
    save_bunnyData.scale = glm::vec3(5.0f);
    GameObject* bunnyObject = SceneUtils::addGameObjectComponent(registry, bunnyEntity, save_bunnyData);

    Mesh* bunnyMesh = ResourceLoader::loadMesh(MODEL_DIR + "stanfordBunny.obj");
    if (bunnyMesh != nullptr) {
        Material* bunnyMaterial = new Material(basicShader);
        bunnyMaterial->albedoColor = glm::vec4(1.0f, 0.5f, 0.31f, 1.0f);
        bunnyMaterial->isDeferred = true;

        Texture* bunnyAlbedoMap = new Texture(TEXTURE_DIR + "uv_map.jpg");
        bunnyMaterial->albedoMap = bunnyAlbedoMap;

        bunnyMesh->material = bunnyMaterial;
        registry.emplace<Mesh*>(bunnyEntity, bunnyMesh);
    }

    for (int row = 0; row < numRows; ++row) {
        entt::entity carEntity = registry.create();
        SceneData save_carData;
        save_carData.name = "Buggy";
        save_carData.position = glm::vec3(6.0f, -2.5f, startZ + row * spacing);
        save_carData.scale = glm::vec3(5.0f);
        GameObject* carObject = SceneUtils::addGameObjectComponent(registry, carEntity, save_carData);
        carObject->addScript<MoveScript>();

        Mesh* carMesh = ResourceLoader::loadMesh(MODEL_DIR + "../obj-assets/data/beetle.obj");
        if (carMesh != nullptr) {
            Material* carMaterial = new Material(basicShader);
            carMaterial->albedoColor = glm::vec4(0.2f, 0.7f, 0.2f, 0.5f);
            carMaterial->isDeferred = false;
            carMesh->material = carMaterial;
            registry.emplace<Mesh*>(carEntity, carMesh);
        }

        // Parent bunny to the first buggy
        if (row == 0) {
            bunnyObject->setParent(carEntity);
        }
    }

    // --------------------- Gold Wall ---------------------
    int numBalls = 3;
    float ballSpace = 6.0f;
    for (int i = 0; i < numBalls; ++i) {
        entt::entity ballEntity = registry.create();

        // Set up the game object data
        SceneData save_goldWall;
        save_goldWall.name = "Gold Ball";
        save_goldWall.position = glm::vec3(-10.0f + i * ballSpace, 1.0f, -10.0f); // Adjust position for each ball
        save_goldWall.eulerAngles = glm::vec3(90, 0, 0);
        save_goldWall.scale = glm::vec3(1.0f); // Adjust size if necessary
        GameObject* goldWallObject = SceneUtils::addGameObjectComponent(registry, ballEntity, save_goldWall);

        // Generate the sphere mesh
        Mesh* wallMesh = MeshGen::createSphere(25, 25);
        if (wallMesh != nullptr) {
            // Create and configure the material
            Material* goldMaterial = new Material(basicShader);
            goldMaterial->albedoColor = glm::vec4(1.0f);
            goldMaterial->albedoMap = new Texture(TEXTURE_DIR + "grime/grime.png");
            goldMaterial->normalMap = new Texture(TEXTURE_DIR + "grime/grime-n.png");
            goldMaterial->metallicMap = new Texture(TEXTURE_DIR + "grime/grime-m.png");
            goldMaterial->roughnessMap = new Texture(TEXTURE_DIR + "grime/grime-r.png");
            goldMaterial->aoMap = new Texture(TEXTURE_DIR + "grime/grime-ao.png");
            // goldMaterial->heightMap = new Texture(TEXTURE_DIR + "grime/grime-h.png");
            // goldMaterial->heightScale = 1.0f;
            goldMaterial->isDeferred = true;
            goldMaterial->uvScale = glm::vec2(2.0f);
            wallMesh->material = goldMaterial;

            // Attach the mesh to the entity
            registry.emplace<Mesh*>(ballEntity, wallMesh);
        }
    }

    // --------------------- Ground Plane ---------------------
    entt::entity planeEntity = registry.create();
    SceneData save_planeData;
    save_planeData.name = "Ground Plane";
    save_planeData.position = glm::vec3(0.0f, -1.1f, 0.0f);
    save_planeData.scale = glm::vec3(1.0f, 1.0f, 1.0f);
    GameObject* planeObject = SceneUtils::addGameObjectComponent(registry, planeEntity, save_planeData);

    Mesh* planeMesh = MeshGen::createPlane(5,5,100,100);
    if (planeMesh != nullptr) {
        Material* planeMaterial = new Material(basicShader);
        planeMaterial->albedoColor = glm::vec4(1.0f);
        planeMaterial->albedoMap = new Texture(TEXTURE_DIR + "grime/grime.png");
        planeMaterial->normalMap = new Texture(TEXTURE_DIR + "grime/grime-n.png");
        planeMaterial->metallicMap = new Texture(TEXTURE_DIR + "grime/grime-m.png");
        planeMaterial->roughnessMap = new Texture(TEXTURE_DIR + "grime/grime-r.png");
        planeMaterial->aoMap = new Texture(TEXTURE_DIR + "grime/grime-ao.png");
        // planeMaterial->heightMap = new Texture(TEXTURE_DIR + "grime/grime-h.png");
        // planeMaterial->heightScale = 1.0f;
        planeMaterial->isDeferred = true;
        planeMaterial->uvScale = glm::vec2(2.0f);
        planeMesh->material = planeMaterial;
        registry.emplace<Mesh*>(planeEntity, planeMesh);
    }

    // --------------------- Diablo Model ---------------------
    entt::entity diabloEntity = registry.create();
    SceneData save_diabloData;
    save_diabloData.name = "Diablo";
    save_diabloData.position = glm::vec3(0.0f, 1.9f, -1.0f);
    save_diabloData.scale = glm::vec3(3.0f);
    GameObject* diabloObject = SceneUtils::addGameObjectComponent(registry, diabloEntity, save_diabloData);
    diabloObject->addScript<MoveScript>();

    Mesh* diabloMesh = ResourceLoader::loadMesh(MODEL_DIR + "/diablo3_pose.obj");
    if (diabloMesh != nullptr) {
        Material* diabloMaterial = new Material(basicShader);
        diabloMaterial->albedoColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        diabloMaterial->isDeferred = true;

        Texture* diabloAlbedoMap = new Texture(TEXTURE_DIR + "diablo/diablo3_pose_diffuse.tga");
        diabloMaterial->albedoMap = diabloAlbedoMap;

        Texture* diabloNormalMap = new Texture(TEXTURE_DIR + "diablo/diablo3_pose_nm_tangent.tga");
        diabloMaterial->normalMap = diabloNormalMap;

        diabloMesh->material = diabloMaterial;
        registry.emplace<Mesh*>(diabloEntity, diabloMesh);
    }

    // --------------------- Dummy Entity (global scripts) ------------------
    entt::entity dummyEntity = registry.create();
    SceneData save_dummyData;
    save_dummyData.name = "GLOBAL SCRIPT HOLDER";
    GameObject* dummyObject = SceneUtils::addGameObjectComponent(registry, dummyEntity, save_dummyData);
    dummyObject->addScript<ViewFrameBuffers>();

    // --------------------- Light Circle ---------------------
    int n = 8;
    float circleRadius = 10.0f;
    float yPosition = 3.0f;
    createLights(3, circleRadius, yPosition, basicShader);
    // createAsteroids(n, circleRadius * 10.0f, 0, 1.0f, basicShader);

    // Gizmo Cube
    // Box dimensions
    int sizeX = 5, sizeY = 5, sizeZ = 5;
    glm::vec3 cubeScale = glm::vec3(1.0f);

    int numCubes = sizeX * sizeY * sizeZ;
    for (int x = 0; x < sizeX; x++) {
        for (int y = 0; y < sizeY; y++) {
            for (int z = 0; z < sizeZ; z++) {
                SceneData data;
                data.position = glm::vec3(x, y, z) * cubeScale;
                data.scale = cubeScale;

                // auto temp = SceneUtils::createGizmo(registry, data, m_wireframeMaterial, GizmoType::CUBE);
            }
        }
    }

    // Finally, update the mesh instance map if any for later use
    updateInstanceMap();
}

void Scene::createLights(int n, float circleRadius, float yPosition, Shader* basicShader) {
    for (int i = 0; i < n; ++i) {
        float angle = i * (360.0f / n);
        float radians = glm::radians(angle);
        float x = circleRadius * std::cos(radians);
        float z = circleRadius * std::sin(radians);

        entt::entity lightEntity = registry.create();

        // Light meta data
        SceneData save_lightData;
        save_lightData.name = "Light(" + std::to_string(i) + ")";
        save_lightData.scale = glm::vec3(0.1f);
        save_lightData.position = glm::vec3(x, yPosition, z);

        // Light game object
        GameObject* lightObject = SceneUtils::addGameObjectComponent(registry, lightEntity, save_lightData);
        lightObject->addScript<CircularRotation>();
        CircularRotation* rotationScript = lightObject->getScript<CircularRotation>();
        if (rotationScript != nullptr) {
            rotationScript->radius = circleRadius;
            rotationScript->center = glm::vec3(0.0f, yPosition, 0.0f);
            rotationScript->rotationSpeed = 0.5f;
        }

        // Light component
        Light lightData;
        glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        switch (i % n) {
            case 0:
                color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); // Soft White
                break;
            case 1:
                color = glm::vec4(1.0f, 0.8f, 0.4f, 1.0f); // Soft Orange
                break;
            case 2:
                color = glm::vec4(0.2f, 0.2f, 0.4f, 1.0f); // Soft Dark Blue
                break;
            default:
                color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); // Soft White (default)
                break;
        }

        lightData.color = color;
        lightData.intensity = 5.0f; // Lower intensity for softer lighting
        lightData.radius = 100.0f;  // Radius of the light
        lightData.type = LightType::Point; // Point light
        lightData.castsShadows = true;   // Enable shadows if necessary
        lightData.isActive = true;       // Ensure the light is active

        SceneUtils::addPointLightComponents(registry, lightEntity, lightData);

        // Cube mesh
        Mesh* lightCube = ResourceLoader::loadMesh(MODEL_DIR + "cube.obj");
        if (lightCube != nullptr) {
            Material* cubeMaterial = new Material(basicShader);
            cubeMaterial->albedoColor = color;
            cubeMaterial->albedoMap = nullptr;
            cubeMaterial->normalMap = nullptr;
            cubeMaterial->isDeferred = false;
            lightCube->material = cubeMaterial;
            registry.emplace<Mesh*>(lightEntity, lightCube);
        }
    }
}

void Scene::createAsteroids(int n, float fieldSize, float minHeight, float maxHeight, Shader* basicShader) {
    Mesh* asteroidMesh = ResourceLoader::loadMesh(MODEL_DIR + "cube.obj");
    if (!asteroidMesh) {
        return;
    }

    Material* asteroidMaterial = new Material(basicShader);
    asteroidMaterial->albedoColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    asteroidMaterial->albedoMap = nullptr;
    asteroidMaterial->normalMap = nullptr;
    asteroidMaterial->isDeferred = true;
    asteroidMesh->material = asteroidMaterial;
    MeshInstance meshInstance = addMeshInstance(asteroidMesh);

    // Randomization setup
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-fieldSize, fieldSize);
    std::uniform_real_distribution<float> heightDist(minHeight, maxHeight);
    std::uniform_real_distribution<float> scaleDist(0.05f, 0.2f);
    std::uniform_real_distribution<float> angleDist(0.0f, 360.0f);
    std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);  // For color
    std::uniform_real_distribution<float> intensityDist(1.0f, 4.0f);  // For intensity
    std::uniform_real_distribution<float> radiusDist(3.0f, 3.0f);  // For radius

    for (int i = 0; i < n; ++i) {
        // Random position
        float x = posDist(gen);
        float y = heightDist(gen);
        float z = posDist(gen);

        // Random rotation
        glm::vec3 randomRotation(angleDist(gen), angleDist(gen), angleDist(gen));

        // Random scale
        float randomScale = scaleDist(gen);

        entt::entity asteroidEntity = registry.create();

        // Meta data
        SceneData save_asteroid;
        save_asteroid.name = "Asteroid(" + std::to_string(i) + ")";
        save_asteroid.scale = glm::vec3(randomScale);
        save_asteroid.position = glm::vec3(x, y, z);
        save_asteroid.eulerAngles = randomRotation;

        // Game object
        GameObject* asteroidObject = SceneUtils::addGameObjectComponent(registry, asteroidEntity, save_asteroid);
        asteroidObject->addScript<BouncingMotion>();

        // Add a point light to each asteroid
        Light asteroidLight;
        asteroidLight.color = glm::vec3(colorDist(gen), colorDist(gen), colorDist(gen));
        asteroidLight.intensity = 25.0f;
        asteroidLight.radius = 50.0f;
        asteroidLight.type = LightType::Point;
        asteroidLight.castsShadows = true;
        asteroidLight.isActive = true;
        SceneUtils::addPointLightComponents(registry, asteroidEntity, asteroidLight);

        registry.emplace<MeshInstance>(asteroidEntity, meshInstance);
    }
}

Camera& Scene::getPrimaryCamera() {
    return registry.get<Camera>(m_primaryCameraEntity);
}

void Scene::setPrimaryCamera(entt::entity cameraEntity) {
    m_primaryCameraEntity = cameraEntity;
}

// Scene instancing
MeshInstance Scene::addMeshInstance(Mesh* mesh) {
    size_t meshIndex = m_meshInstances.size();
    m_meshInstances.push_back(mesh);

    MeshInstance newInstance;
    newInstance.id = meshIndex;
    return newInstance;
}

void Scene::updateInstanceMap() {
    // Handle instance count updates (less frequent)
    if (m_instanceCountsDirty) {
        // Reset counts while keeping map structure
        for (auto& [id, count] : m_instanceCounts) {
            count = 0;
        }

        // Count instances
        registry.view<MeshInstance>().each([&](const MeshInstance& instance) {
            m_instanceCounts[instance.id]++;
        });

        // Resize vectors based on new counts
        for (const auto& [id, count] : m_instanceCounts) {
            if (m_instanceMap.find(id) == m_instanceMap.end()) {
                m_instanceMap[id] = std::vector<glm::mat4>(count);
            } else {
                m_instanceMap[id].resize(count);
            }
        }

        m_instanceCountsDirty = false;
    }

    // Reset indices at the start of each update
    for (auto& [id, index] : m_currentIndices) {
        index = 0;
    }

    // Collect matrices in a single view iteration
    registry.view<MeshInstance, ModelMatrix>().each([&](const MeshInstance& instance, const ModelMatrix& modelMatrix) {
        size_t id = instance.id;
        size_t& idx = m_currentIndices[id];
        m_instanceMap[id][idx] = modelMatrix.matrix;
        idx++;
    });
}
