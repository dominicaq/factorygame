#include "scene.h"

#include <stdexcept>

#include "CircularRotation.h"
#include "FreeCamera.h"
#include "MoveScript.h"
#include "ViewFramebuffers.h"

/*
* Scene Management
*/

// TODO: In the future, this will be a virtual func for multiple user created scenes
void Scene::loadScene() {
    // ------------------------ Setup Camera ------------------------
    entt::entity cameraEntity = registry.create();
    MetaData meta_cameraData;
    meta_cameraData.position = glm::vec3(4.0f, 0.21f, 4.04f);
    meta_cameraData.eulerAngles = glm::vec3(-2.38f, 239.0f, 0.0f);
    GameObject* cameraObject = addGameObjectComponent(registry, cameraEntity, meta_cameraData);
    cameraObject->addScript<FreeCamera>();

    Camera cameraComponent(cameraEntity, registry);
    registry.emplace<Camera>(cameraEntity, std::move(cameraComponent));
    setPrimaryCamera(cameraEntity);

    // ------------------------ Shader Setup ------------------------
    std::string vertexPath = SHADER_DIR + "default.vs";
    std::string fragmentPath = SHADER_DIR + "default.fs";
    Shader* basicShader = new Shader(vertexPath, fragmentPath);

    // --------------------- Beetle Car ---------------------
    entt::entity carEntity = registry.create();
    MetaData meta_carData;
    meta_carData.position = glm::vec3(6.0f, -2.5f, -0.5f);
    meta_carData.scale = glm::vec3(5.0f);
    GameObject* carObject = addGameObjectComponent(registry, carEntity, meta_carData);
    carObject->addScript<MoveScript>();

    Mesh* carMesh = ResourceLoader::loadMesh(MODEL_DIR + "../obj-assets/data/beetle.obj");
    if (carMesh != nullptr) {
        Material* carMaterial = new Material(basicShader);
        carMaterial->albedoColor = glm::vec3(0.2f, 0.7f, 0.2f);
        carMaterial->isDeferred = true;
        carMesh->material = carMaterial;
        registry.emplace<Mesh*>(carEntity, carMesh);
    }

    // --------------------- Stanford Bunny Model ---------------------
    entt::entity bunnyEntity = registry.create();
    MetaData meta_bunnyData;
    meta_bunnyData.position = glm::vec3(6.0f, -0.5f, 0.0f);
    meta_bunnyData.scale = glm::vec3(5.0f);
    GameObject* bunnyObject = addGameObjectComponent(registry, bunnyEntity, meta_bunnyData);
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
    MetaData meta_diabloData;
    meta_diabloData.position = glm::vec3(0.0f, 0.0f, -1.0f);
    meta_diabloData.scale = glm::vec3(2.0f);
    GameObject* diabloObject = addGameObjectComponent(registry, diabloEntity, meta_diabloData);
    diabloObject->addScript<MoveScript>();

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
    MetaData meta_dummyData;
    GameObject* dummyObject = addGameObjectComponent(registry, dummyEntity, meta_dummyData);
    dummyObject->addScript<ViewFrameBuffers>();

    // --------------------- Light Circle ---------------------
    int n = 3;
    float circleRadius = 5.0f;
    float yPosition = 2.0f;

    for (int i = 0; i < n; ++i) {
        float angle = i * (360.0f / n);
        float radians = glm::radians(angle);
        float x = circleRadius * std::cos(radians);
        float z = circleRadius * std::sin(radians);

        entt::entity lightEntity = registry.create();
        MetaData meta_lightData;
        meta_lightData.scale = glm::vec3(0.1f);
        meta_lightData.position = glm::vec3(x, yPosition, z);
        GameObject* lightObject = addGameObjectComponent(registry, lightEntity, meta_lightData);
        lightObject->addScript<CircularRotation>();
        CircularRotation* rotationScript = lightObject->getScript<CircularRotation>();
        if (rotationScript != nullptr) {
            rotationScript->radius = circleRadius;
            rotationScript->center = glm::vec3(0.0f, yPosition, 0.0f);
            rotationScript->rotationSpeed = 1.5f;
        }

        Light lightData;
        glm::vec3 color;
        switch (i % n) {
            case 0: color = glm::vec3(1.0f, 0.0f, 0.0f); break; // Red
            case 1: color = glm::vec3(0.0f, 1.0f, 0.0f); break; // Green
            case 2: color = glm::vec3(0.0f, 0.0f, 1.0f); break; // Blue
            default: color = glm::vec3(1.0f); break;            // White
        }
        lightData.color = color;
        lightData.intensity = 1.0f;
        lightData.radius = 1.0f;
        lightData.type = LightType::Point;
        lightData.castsShadows = false;
        lightData.isActive = true;
        registry.emplace<Light>(lightEntity, lightData);

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

Camera& Scene::getPrimaryCamera() {
    return registry.get<Camera>(m_primaryCameraEntity);
}

void Scene::setPrimaryCamera(entt::entity cameraEntity) {
    m_primaryCameraEntity = cameraEntity;
}

/*
* Component Helpers
*/
GameObject* Scene::addGameObjectComponent(entt::registry& registry, entt::entity entity, const MetaData& data) {
    if (!registry.valid(entity)) {
        return nullptr;
    }

    registry.emplace<Position>(entity, data.position);
    registry.emplace<EulerAngles>(entity, data.eulerAngles);
    registry.emplace<Rotation>(entity, glm::quat(glm::radians(data.eulerAngles)));
    registry.emplace<Scale>(entity, data.scale);
    registry.emplace<ModelMatrix>(entity);

    return &registry.emplace<GameObject>(entity, entity, registry);
}
