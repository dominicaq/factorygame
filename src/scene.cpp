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
    // ------------------------ Setup Camera ------------------------
    entt::entity cameraEntity = registry.create();
    SceneData save_cameraData;
    save_cameraData.name = "Main Camera";
    save_cameraData.position = glm::vec3(4.0f, 0.21f, 4.04f);
    save_cameraData.eulerAngles = glm::vec3(-2.38f, 239.0f, 0.0f);
    GameObject* cameraObject = addGameObjectComponent(registry, cameraEntity, save_cameraData);
    cameraObject->addScript<FreeCamera>();

    Camera cameraComponent(cameraEntity, registry);
    registry.emplace<Camera>(cameraEntity, cameraComponent);
    setPrimaryCamera(cameraEntity);

    // ------------------------ Shader Setup ------------------------
    std::string vertexPath = SHADER_DIR + "default.vs";
    std::string fragmentPath = SHADER_DIR + "default.fs";
    Shader* basicShader = new Shader(vertexPath, fragmentPath);

    // --------------------- Beetle Car ---------------------
    entt::entity carEntity = registry.create();
    SceneData save_carData;
    save_carData.name = "Buggy";
    save_carData.position = glm::vec3(6.0f, -2.5f, -0.5f);
    save_carData.scale = glm::vec3(5.0f);
    GameObject* carObject = addGameObjectComponent(registry, carEntity, save_carData);
    carObject->addScript<MoveScript>();

    Mesh* carMesh = ResourceLoader::loadMesh(MODEL_DIR + "../obj-assets/data/beetle.obj");
    if (carMesh != nullptr) {
        Material* carMaterial = new Material(basicShader);
        carMaterial->albedoColor = glm::vec3(0.2f, 0.7f, 0.2f);
        carMaterial->isDeferred = true;
        carMesh->material = carMaterial;
        registry.emplace<Mesh*>(carEntity, carMesh);
    }

    // --------------------- Ground Plane ---------------------
    entt::entity planeEntity = registry.create();
    SceneData save_planeData;
    save_planeData.name = "Ground Plane";
    save_planeData.position = glm::vec3(0.0f, -1.1f, 0.0f);
    save_planeData.scale = glm::vec3(30.0f, 0.1f, 30.0f);
    GameObject* planeObject = addGameObjectComponent(registry, planeEntity, save_planeData);

    Mesh* planeMesh = ResourceLoader::loadMesh(MODEL_DIR + "/cube.obj");
    if (planeMesh != nullptr) {
        Material* planeMaterial = new Material(basicShader);
        planeMaterial->albedoColor = glm::vec3(1.0f);
        planeMaterial->albedoMap = new Texture(TEXTURE_DIR + "dev.jpg");
        planeMaterial->isDeferred = true;
        planeMesh->material = planeMaterial;
        registry.emplace<Mesh*>(planeEntity, planeMesh);
    }

    // --------------------- Stanford Bunny Model ---------------------
    entt::entity bunnyEntity = registry.create();
    SceneData save_bunnyData;
    save_bunnyData.name = "Bunny";
    save_bunnyData.position = glm::vec3(6.0f, -0.5f, 0.0f);
    save_bunnyData.scale = glm::vec3(5.0f);
    GameObject* bunnyObject = addGameObjectComponent(registry, bunnyEntity, save_bunnyData);
    // Parent bunny to car
    bunnyObject->setParent(carEntity);

    Mesh* bunnyMesh = ResourceLoader::loadMesh(MODEL_DIR + "stanfordBunny.obj");
    if (bunnyMesh != nullptr) {
        Material* bunnyMaterial = new Material(basicShader);
        bunnyMaterial->albedoColor = glm::vec3(1.0f, 0.5f, 0.31f);
        bunnyMaterial->isDeferred = true;

        Texture* bunnyAlbedoMap = new Texture(TEXTURE_DIR + "uv_map.jpg");
        bunnyMaterial->albedoMap = bunnyAlbedoMap;

        bunnyMesh->material = bunnyMaterial;
        registry.emplace<Mesh*>(bunnyEntity, bunnyMesh);
    }

    // --------------------- Diablo Model ---------------------
    entt::entity diabloEntity = registry.create();
    SceneData save_diabloData;
    save_diabloData.name = "Diablo";
    save_diabloData.position = glm::vec3(0.0f, -0.1f, -1.0f);
    save_diabloData.scale = glm::vec3(1.0f);
    GameObject* diabloObject = addGameObjectComponent(registry, diabloEntity, save_diabloData);
    // diabloObject->addScript<MoveScript>();

    Mesh* diabloMesh = ResourceLoader::loadMesh(MODEL_DIR + "diablo3_pose.obj");
    if (diabloMesh != nullptr) {
        Material* diabloMaterial = new Material(basicShader);
        diabloMaterial->albedoColor = glm::vec3(0.7f, 0.7f, 0.7f);
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
    GameObject* dummyObject = addGameObjectComponent(registry, dummyEntity, save_dummyData);
    dummyObject->addScript<ViewFrameBuffers>();

    // --------------------- Light Circle ---------------------
    int n = 100000;
    float circleRadius = 4.0f;
    float yPosition = 0.0f;
    createLights(4, circleRadius, yPosition + 5.0f, basicShader);
    createAsteroids(n, circleRadius * 10.0f, 0, 200, basicShader);

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
        GameObject* lightObject = addGameObjectComponent(registry, lightEntity, save_lightData);
        lightObject->addScript<CircularRotation>();
        CircularRotation* rotationScript = lightObject->getScript<CircularRotation>();
        if (rotationScript != nullptr) {
            rotationScript->radius = circleRadius;
            rotationScript->center = glm::vec3(0.0f, yPosition, 0.0f);
            rotationScript->rotationSpeed = 0.5f;
        }

        // Light component
        Light lightData;
        glm::vec3 color;
        switch (i % n) {
            case 0: color = glm::vec3(1.0f, 1.0f, 1.0f); break; // White
            case 1: color = glm::vec3(1.0f, 0.0f, 0.0f); break; // Red
            case 2: color = glm::vec3(0.0f, 1.0f, 0.0f); break; // Green
            default: color = glm::vec3(0.0f, 0.0f, 1.0f); break; // Blue
        }
        lightData.color = color;
        lightData.intensity = 1.0f;
        lightData.radius = 20.0f;
        lightData.type = LightType::Point;
        lightData.castsShadows = true;
        lightData.isActive = true;
        addPointLightComponents(registry, lightEntity, lightData);

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
    asteroidMaterial->albedoColor = glm::vec3(0.5f, 0.5f, 0.5f); // Grayish color
    asteroidMaterial->albedoMap = nullptr;
    asteroidMaterial->normalMap = nullptr;
    asteroidMaterial->isDeferred = true;
    asteroidMesh->material = asteroidMaterial;
    MeshInstance meshInstance = addMeshInstance(asteroidMesh);

    // Randomization setup
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-fieldSize, fieldSize); // Random X, Z in a cube area
    std::uniform_real_distribution<float> heightDist(minHeight, maxHeight); // Random Y height
    std::uniform_real_distribution<float> scaleDist(0.05f, 0.2f);  // Random scale range
    std::uniform_real_distribution<float> angleDist(0.0f, 360.0f); // Random rotation

    for (int i = 0; i < n; ++i) {
        // Random position
        float x = posDist(gen);
        float y = heightDist(gen); // Varies between minHeight and maxHeight
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
        save_asteroid.eulerAngles = randomRotation; // Assuming your system supports rotations

        // Game object
        GameObject* asteroidObject = addGameObjectComponent(registry, asteroidEntity, save_asteroid);
        asteroidObject->addScript<BouncingMotion>();

        registry.emplace<MeshInstance>(asteroidEntity, meshInstance);
    }
}

Camera& Scene::getPrimaryCamera() {
    return registry.get<Camera>(m_primaryCameraEntity);
}

void Scene::setPrimaryCamera(entt::entity cameraEntity) {
    m_primaryCameraEntity = cameraEntity;
}

/*
* Component Helpers
*/
void Scene::addLightComponents(entt::registry& registry, entt::entity entity, Light lightData) {
    registry.emplace<Light>(entity, lightData);
    registry.emplace<LightSpaceMatrix>(entity);
}

void Scene::addPointLightComponents(entt::registry& registry, entt::entity entity, Light lightData) {
    registry.emplace<Light>(entity, lightData);
    registry.emplace<LightSpaceMatrixCube>(entity);
}

GameObject* Scene::addGameObjectComponent(entt::registry& registry, entt::entity entity, const SceneData& data) {
    if (!registry.valid(entity)) {
        return nullptr;
    }

    // Meta data
    registry.emplace<MetaData>(entity, data.name);

    // Transform
    registry.emplace<Position>(entity, data.position);
    registry.emplace<EulerAngles>(entity, data.eulerAngles);
    registry.emplace<Rotation>(entity, glm::quat(glm::radians(data.eulerAngles)));
    registry.emplace<Scale>(entity, data.scale);
    registry.emplace<ModelMatrix>(entity);

    return &registry.emplace<GameObject>(entity, entity, registry);
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
        size_t& idx = m_currentIndices[id]; // Reference to avoid double lookup
        m_instanceMap[id][idx] = modelMatrix.matrix;
        idx++;
    });
}
