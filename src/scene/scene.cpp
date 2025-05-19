#include "scene.h"
#include <random>
#include <chrono>

// User scripts
#include "CircularRotation.h"
#include "FreeCamera.h"
#include "MoveScript.h"
#include "BouncingMotion.h"
#include "ViewFramebuffers.h"
#include "ScaleScript.h"

/*
* Scene Management
*/

// TODO: In the future, this will be user generated through UI, not code.
void Scene::loadScene() {
    // ------------------------ Wireframe Setup ------------------------
    Shader* wireframeShader = new Shader(SHADER_DIR + "debug/wireframe.vs", SHADER_DIR + "debug/wireframe.fs");
    m_wireframeMaterial = new Material(wireframeShader);
    m_wireframeMaterial->albedoColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

    // ------------------------ Skybox Setup ------------------------
    std::vector<std::string> skyboxPaths = {
        ASSET_DIR "textures/skyboxes/bspace/1.png",
        ASSET_DIR "textures/skyboxes/bspace/3.png",
        ASSET_DIR "textures/skyboxes/bspace/5.png",
        ASSET_DIR "textures/skyboxes/bspace/6.png",
        ASSET_DIR "textures/skyboxes/bspace/2.png",
        ASSET_DIR "textures/skyboxes/bspace/4.png"
    };
    loadSkyBox(skyboxPaths);

    // ------------------------ Setup Camera ------------------------
    entt::entity cameraEntity = registry.create();
    SceneData save_cameraData;
    save_cameraData.name = "Main Camera";
    save_cameraData.position = glm::vec3(0.0f, 10.0f, 0.0f);
    save_cameraData.eulerAngles = glm::vec3(0.0f, 0.0f, 0.0f);
    GameObject* cameraObject = SceneUtils::addGameObjectComponent(registry, cameraEntity, save_cameraData);
    cameraObject->addScript<FreeCamera>();

    // Toggle the flash light with F key
    Light flashLight;
    flashLight.color = glm::vec3(1.0f);
    flashLight.intensity = 5.0f;
    flashLight.castShadow = true;
    flashLight.isActive = false;

    flashLight.type = LightType::Spot;
    flashLight.spot.innerCutoff = cos(glm::radians(3.0f));
    flashLight.spot.outerCutoff = cos(glm::radians(30.0f));
    flashLight.spot.range = 25.0f;
    SceneUtils::addLightComponents(registry, cameraEntity, flashLight);

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

        bunnyMaterial->albedoMap = new Texture(TEXTURE_DIR + "uv_map.jpg");

        bunnyMesh->material = bunnyMaterial;
        // registry.emplace<Mesh*>(bunnyEntity, bunnyMesh);
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
            // registry.emplace<Mesh*>(carEntity, carMesh);
        }

        // Parent bunny to the first buggy
        if (row == 0) {
            bunnyObject->setParent(carEntity);
        }
    }

    // --------------------- Gold Wall ---------------------
    int numBalls = 0;
    float ballSpace = 6.0f;
    for (int i = 0; i < numBalls; ++i) {
        entt::entity ballEntity = registry.create();

        // Set up the game object data
        SceneData save_goldWall;
        save_goldWall.name = "Gold Ball";
        save_goldWall.position = glm::vec3(-10.0f + i * ballSpace, 1.0f, -10.0f);
        save_goldWall.eulerAngles = glm::vec3(90, 0, 0);
        save_goldWall.scale = glm::vec3(2.0f);
        GameObject* goldWallObject = SceneUtils::addGameObjectComponent(registry, ballEntity, save_goldWall);

        // Generate the sphere mesh
        Mesh* wallMesh = MeshGen::createSphere(50, 50);
        if (wallMesh != nullptr) {
            // Create and configure the material
            Material* ballMat = new Material(basicShader);
            ballMat->albedoColor = glm::vec4(1.0f);
            ballMat->albedoMap = new Texture(TEXTURE_DIR + "gold/gold.png");
            ballMat->normalMap = new Texture(TEXTURE_DIR + "gold/gold-n.png");
            ballMat->heightMap = new Texture(TEXTURE_DIR + "gold/gold-h.ong");
            ballMat->heightScale = 0.01f;
            ballMat->isDeferred = true;
            ballMat->uvScale = glm::vec2(1.0f);
            wallMesh->material = ballMat;

            // Attach the mesh to the entity
            registry.emplace<Mesh*>(ballEntity, wallMesh);
        }
    }

    // --------------------- Ground Plane ---------------------
    entt::entity planeEntity = registry.create();
    SceneData save_planeData;
    save_planeData.name = "Ground Plane";
    save_planeData.position = glm::vec3(0.0f, -1.1f, 0.0f);
    save_planeData.eulerAngles = glm::vec3(0.0f, 0.0f, 0.0f);
    save_planeData.scale = glm::vec3(1.0f, 1.0f, 1.0f);
    GameObject* planeObject = SceneUtils::addGameObjectComponent(registry, planeEntity, save_planeData);

    Mesh* planeMesh = MeshGen::createPlane(2,2,200,200);
    if (planeMesh != nullptr) {
        Material* planeMaterial = new Material(basicShader);
        planeMaterial->albedoColor = glm::vec4(1.0f);
        planeMaterial->albedoMap = new Texture(TEXTURE_DIR + "tiles/tiles.tga");
        planeMaterial->normalMap = new Texture(TEXTURE_DIR + "tiles/tiles-n.tga");
        planeMaterial->heightMap = new Texture(TEXTURE_DIR + "tiles/tiles-h.tga");
        planeMaterial->heightScale = 0.05f;
        planeMaterial->isDeferred = true;
        planeMaterial->uvScale = glm::vec2(4.0f);
        planeMesh->material = planeMaterial;
        registry.emplace<Mesh*>(planeEntity, planeMesh);
    }

    // --------------------- Diablo Model ---------------------
    entt::entity diabloEntity = registry.create();
    SceneData save_diabloData;
    save_diabloData.name = "Diablo";
    save_diabloData.position = glm::vec3(0.0f, 3.0f, 0.0f);
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

    /*
    * glTF Game Objects
    */
    auto gltfLoadStart = std::chrono::high_resolution_clock::now();
    auto gltfLoadEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> gltfLoadElasped;

    gltfLoadStart = std::chrono::high_resolution_clock::now();
    GameObject* helmetObj = SceneUtils::createMeshGameObject(registry, basicShader, ASSET_DIR "gltf-assets/Models/DamagedHelmet/glTF/DamagedHelmet.gltf");
    if (helmetObj) {
        helmetObj->setScale(glm::vec3(1.0f));
        helmetObj->setPosition(glm::vec3(0.0f, 5.0f, -5.0f));
        helmetObj->setEuler(glm::vec3(90,0,0));
        // helmetObj->addScript<MoveScript>();
    }
    gltfLoadEnd = std::chrono::high_resolution_clock::now();
    gltfLoadElasped = gltfLoadEnd - gltfLoadStart;
    std::cout << "createMeshGameObject(Helmet) took " << gltfLoadElasped.count() << " seconds.\n";

    GameObject* dragonObj = SceneUtils::createMeshGameObject(registry, basicShader, ASSET_DIR "dragon/dragon.gltf");
    if (dragonObj) {
        dragonObj->setScale(glm::vec3(0.5f));
        dragonObj->setPosition(glm::vec3(10.0f, -0.5f, 2.5f));
    }

    GameObject* laternObj = SceneUtils::createMeshGameObject(registry, basicShader, ASSET_DIR "gltf-assets/Models/Lantern/glTF/Lantern.gltf");
    if (laternObj) {
        laternObj->setScale(glm::vec3(0.5f));
        laternObj->setPosition(glm::vec3(10.0f, -0.25f, -2.5f));
        laternObj->addScript<MoveScript>();
    }

    GameObject* boomBox = SceneUtils::createMeshGameObject(registry, basicShader, ASSET_DIR "gltf-assets/Models/BoomBox/glTF/BoomBox.gltf");
    if (boomBox) {
        boomBox->setScale(glm::vec3(100.0f));
        boomBox->setPosition(glm::vec3(0.0f, 15.0f, -5.0f));
    }

    gltfLoadStart = std::chrono::high_resolution_clock::now();
    GameObject* romanObj = SceneUtils::createMeshGameObject(registry, basicShader, ASSET_DIR "gltf-assets/Models/Sponza/glTF/Sponza.gltf");
    if (romanObj) {
        romanObj->setScale(glm::vec3(0.0003f));
        romanObj->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    }
    gltfLoadEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = gltfLoadEnd - gltfLoadStart;
    std::cout << "createMeshGameObject(Sponza) took " << elapsed.count() << " seconds.\n";

    // --------------------- Dummy Entity (global scripts) ------------------
    entt::entity dummyEntity = registry.create();
    SceneData save_dummyData;
    save_dummyData.name = "GLOBAL SCRIPT HOLDER";
    GameObject* dummyObject = SceneUtils::addGameObjectComponent(registry, dummyEntity, save_dummyData);
    dummyObject->addScript<ViewFrameBuffers>();

    // --------------------- Light Circle ---------------------
    int n = 0;
    float circleRadius = 5.0f;
    float yPosition = 10.0f;
    createSuns(1, 50.0f, 50.0f, basicShader);

    createSpotLights(2, circleRadius, yPosition, basicShader);

    // Light balls
    createSpheres(n, circleRadius * 10.0f, 0, 1.0f, basicShader, true);

    // Normal balls
    createSpheres(10000, circleRadius * 10.0f, 0, 1.0f, basicShader, false);

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

void Scene::createSuns(int n, float circleRadius, float yPosition, Shader* basicShader) {
    for (int i = 0; i < n; ++i) {
        float angle = i * (360.0f / n);
        float radians = glm::radians(angle);
        float x = circleRadius * std::cos(radians);
        float z = circleRadius * std::sin(radians);

        entt::entity lightEntity = registry.create();

        // Light meta data
        SceneData save_lightData;
        save_lightData.name = "sun(" + std::to_string(i) + ")";
        save_lightData.scale = glm::vec3(1.0f);
        save_lightData.position = glm::vec3(x, yPosition, z);

        // Light game object
        GameObject* lightObject = SceneUtils::addGameObjectComponent(registry, lightEntity, save_lightData);
        lightObject->addScript<CircularRotation>();
        CircularRotation* rotationScript = lightObject->getScript<CircularRotation>();
        if (rotationScript != nullptr) {
            rotationScript->radius = circleRadius;
            rotationScript->center = glm::vec3(0.0f, yPosition, 0.0f);
            rotationScript->rotationSpeed = 0.05f;
        }

        // Light component
        Light lightData;
        glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        switch (i+2 % 3) {
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
        lightData.intensity = 15.0f;
        lightData.castShadow = true;
        lightData.isActive = true;

        // Light type properties
        // TODO: Bad shadow map quality. need to improve it
        lightData.type = LightType::Directional;
        lightData.directional.shadowOrthoSize = 20.0f;

        SceneUtils::addLightComponents(registry, lightEntity, lightData);

        // Cube mesh
        Mesh* lightCube = MeshGen::createSphere(30, 30);
        if (lightCube != nullptr) {
            Material* cubeMaterial = new Material(basicShader);
            cubeMaterial->albedoColor = color;
            cubeMaterial->albedoMap = nullptr;
            cubeMaterial->normalMap = nullptr;
            cubeMaterial->isDeferred = false;
            lightCube->material = cubeMaterial;
            // registry.emplace<Mesh*>(lightEntity, lightCube);
        }
    }
}

void Scene::createSpotLights(int n, float circleRadius, float yPosition, Shader* basicShader) {
    for (int i = 0; i < n; ++i) {
        float angle = i * (360.0f / n);
        float radians = glm::radians(angle);
        float x = circleRadius * std::cos(radians);
        float z = circleRadius * std::sin(radians);

        entt::entity lightEntity = registry.create();

        // Light meta data
        SceneData save_lightData;
        save_lightData.name = "Light(" + std::to_string(i) + ")";
        save_lightData.scale = glm::vec3(0.25f);
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
        lightData.intensity = 5.0f;
        lightData.castShadow = true;
        lightData.isActive = true;

        // Light type properties
        lightData.type = LightType::Spot;
        // lightData.point.radius = 35.0f;
        lightData.spot.innerCutoff = cos(glm::radians(3.0f));
        lightData.spot.outerCutoff = cos(glm::radians(30.0f));
        lightData.spot.range = 50.0f;

        SceneUtils::addLightComponents(registry, lightEntity, lightData);

        // Cube mesh
        Mesh* lightCube = ResourceLoader::loadMesh(MODEL_DIR + "/spotlight.obj");
        if (lightCube != nullptr) {
            Material* cubeMaterial = new Material(basicShader);
            cubeMaterial->albedoColor = glm::vec4(1.0f);
            cubeMaterial->albedoMap = new Texture(TEXTURE_DIR + "gold/gold.png");
            cubeMaterial->normalMap = nullptr;
            cubeMaterial->isDeferred = true;
            lightCube->material = cubeMaterial;
            registry.emplace<Mesh*>(lightEntity, lightCube);
        }
    }
}

void Scene::createSpheres(int n, float fieldSize, float minHeight, float maxHeight, Shader* basicShader, bool litSpheres) {
    Mesh* asteroidMesh = MeshGen::createSphere(25, 25);
    if (!asteroidMesh) {
        return;
    }

    Material* asteroidMaterial = new Material(basicShader);
    asteroidMaterial->albedoColor = glm::vec4(1.0f);
    asteroidMaterial->albedoMap = new Texture(TEXTURE_DIR + "gold/gold.png");
    asteroidMaterial->normalMap = new Texture(TEXTURE_DIR + "gold/gold-n.png");
    asteroidMesh->material = asteroidMaterial;
    asteroidMaterial->isDeferred = true;
    MeshInstance meshInstance = addMeshInstance(asteroidMesh);

    // Randomization setup
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-fieldSize, fieldSize);
    std::uniform_real_distribution<float> heightDist(minHeight, maxHeight);
    std::uniform_real_distribution<float> scaleDist(0.25f, 0.25f);
    std::uniform_real_distribution<float> angleDist(0.0f, 360.0f);
    std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);  // For color
    std::uniform_real_distribution<float> intensityDist(1.0f, 4.0f);  // For intensity
    std::uniform_real_distribution<float> radiusDist(3.0f, 3.0f);  // For radius

    for (int i = 0; i < n; ++i) {
        // Random position
        float x = posDist(gen);
        float y = heightDist(gen) + 1.0f;
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

        // Add a point light to each asteroid
        if (litSpheres) {
            Light asteroidLight;
            asteroidLight.color = glm::vec3(colorDist(gen), colorDist(gen), colorDist(gen));
            asteroidLight.intensity = 5.0f;
            asteroidLight.point.radius = 30.0f;
            asteroidLight.type = LightType::Point;
            asteroidLight.castShadow = true;
            asteroidLight.isActive = true;
            SceneUtils::addLightComponents(registry, asteroidEntity, asteroidLight);
        }

        // Game object
        GameObject* asteroidObject = SceneUtils::addGameObjectComponent(registry, asteroidEntity, save_asteroid);
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

void Scene::loadSkyBox(const std::vector<std::string>& skyboxFilePaths) {
    // Ensure we have exactly 6 faces for the skybox
    if (skyboxFilePaths.size() != 6) {
        std::cerr << "Skybox requires exactly 6 texture paths\n";
        return;
    }

    // Load the cubemap textures from the provided file paths
    m_skyboxHandle = CubeMap::createFromImages(skyboxFilePaths);
}

// Scene instancing
MeshInstance Scene::addMeshInstance(Mesh* mesh) {
    uint8_t meshIndex = (uint8_t)m_meshInstances.size();
    m_meshInstances.push_back(mesh);

    MeshInstance newInstance;
    newInstance.id = meshIndex;
    return newInstance;
}

void Scene::updateInstanceMap() {
    // Handle instance count updates (less frequent)
    if (m_instanceCountsDirty) {
        // Clear and count in one pass
        for (auto& [id, count] : m_instanceCounts) {
            count = 0;
        }

        registry.view<MeshInstance>().each([&](const MeshInstance& instance) {
            m_instanceCounts[instance.id]++;
        });

        // Resize vectors based on new counts
        for (auto& [id, count] : m_instanceCounts) {
            auto& instanceVector = m_instanceMap[id];
            instanceVector.resize(count);
            // Initialize current index for this ID while we're here
            m_currentIndices[id] = 0;
        }

        m_instanceCountsDirty = false;
    } else {
        // Only reset indices if we didn't do it in the count update
        for (auto& [id, index] : m_currentIndices) {
            index = 0;
        }
    }

    auto view = registry.view<MeshInstance, ModelMatrix>();
    view.each([&](const auto& instance, const auto& modelMatrix) {
        const size_t id = instance.id;
        auto& instanceVector = m_instanceMap[id];
        size_t& idx = m_currentIndices[id];

        // Safety check in case something got out of sync
        if (idx < instanceVector.size()) {
            instanceVector[idx++] = modelMatrix.matrix;
        }
    });
}
