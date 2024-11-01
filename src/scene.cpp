#include "scene.h"

#include <stdexcept>

#include "MoveScript.h"
#include "ViewFramebuffers.h"
#include "FreeCamera.h"

void loadScene(Scene& scene, entt::registry& registry, LightSystem& lightSystem) {
    // ------------------------ Setup Camera ------------------------
    entt::entity cameraEntity = registry.create();
    addTransformComponents(registry, cameraEntity,
                                      glm::vec3(4.0f, 0.21f, 4.04f),
                                      glm::vec3(-2.38f, 239.0f, 0.0f));

    Camera cameraComponent(cameraEntity, registry);
    registry.emplace<Camera>(cameraEntity, std::move(cameraComponent));

    GameObject* cameraObject = addGameObjectComponent(registry, cameraEntity);
    cameraObject->addScript<FreeCamera>();

    // Track the camera entity and set it as primary
    scene.cameraEntities.push_back(cameraEntity);
    scene.primaryCameraEntity = cameraEntity;

    // ------------------------ Shader Setup ------------------------
    std::string vertexPath = SHADER_DIR + "default.vs";
    std::string fragmentPath = SHADER_DIR + "default.fs";
    Shader* basicShader = new Shader(vertexPath, fragmentPath);

    // --------------------- Stanford Bunny Model ---------------------
    entt::entity bunnyEntity = registry.create();
    addTransformComponents(registry, bunnyEntity,
                                      glm::vec3(0.0f, -0.5f, 0.0f),
                                      glm::vec3(0.0f),
                                      glm::vec3(5.0f));

    Mesh* bunnyMesh = ResourceLoader::loadMesh(MODEL_DIR + "stanfordBunny.obj");
    if (bunnyMesh != nullptr) {
        Material* bunnyMaterial = new Material(basicShader);
        bunnyMaterial->albedoColor = glm::vec3(1.0f, 0.5f, 0.31f);
        bunnyMaterial->isDeferred = true;

        Texture* bunnyAlbedoMap = new Texture(TEXTURE_DIR + "uv_map.jpg");
        bunnyMaterial->albedoMap = bunnyAlbedoMap;

        bunnyMesh->material = bunnyMaterial;
        registry.emplace<Mesh*>(bunnyEntity, bunnyMesh);

        GameObject* bunnyObject = addGameObjectComponent(registry, bunnyEntity);
        bunnyObject->addScript<MoveScript>();
    }

    // --------------------- Cube as Child of Bunny ---------------------
    entt::entity cubeEntity = registry.create();
    addTransformComponents(registry, cubeEntity,
                                      glm::vec3(0.0f, 2.0f, 0.0f),
                                      glm::vec3(0.0f),
                                      glm::vec3(0.5f));

    Mesh* cubeMesh = ResourceLoader::loadMesh(MODEL_DIR + "cube.obj");
    if (cubeMesh != nullptr) {
        Material* cubeMaterial = new Material(basicShader);
        cubeMaterial->albedoColor = glm::vec3(0.2f, 0.7f, 0.2f);
        cubeMaterial->isDeferred = false;
        cubeMesh->material = cubeMaterial;
        registry.emplace<Mesh*>(cubeEntity, cubeMesh);

        GameObject* cubeObject = addGameObjectComponent(registry, cubeEntity);
        cubeObject->setParent(bunnyEntity);
    }

    // --------------------- Diablo Model ---------------------
    entt::entity diabloEntity = registry.create();
    addTransformComponents(registry, diabloEntity,
                                      glm::vec3(2.0f, 0.0f, -1.0f),
                                      glm::vec3(0.0f),
                                      glm::vec3(2.0f));

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

        GameObject* diabloObject = addGameObjectComponent(registry, diabloEntity);
        diabloObject->addScript<MoveScript>();
    }

    // --------------------- Dummy Entity (global scripts) ------------------
    entt::entity dummyEntity = registry.create();
    GameObject* dummyObject = addGameObjectComponent(registry, dummyEntity);
    dummyObject->addScript<ViewFrameBuffers>();

    // --------------------- Light System ---------------------
    entt::entity lightEntity = registry.create();
    lightSystem.addLight(
        glm::vec3(-20.0f, 0.0f, 0.0f),
        glm::vec3(0.2f, 0.2f, 0.2f),
        1.0f,
        3.0f,
        false,
        false
    );

    entt::entity lightEntity2 = registry.create();
    lightSystem.addLight(
        glm::vec3(-2.0f, 2.5f, 2.0f),
        glm::vec3(0.0f, 0.0f, 0.5f),
        10.0f,
        1.0f,
        false,
        false
    );
}

Camera& getPrimaryCamera(const Scene& scene, entt::registry& registry) {
    return registry.get<Camera>(scene.primaryCameraEntity);
}

/*
* Component Helpers
*/
bool hasTransformComponents(entt::registry& registry, entt::entity entity) {
    return registry.all_of<ModelMatrix, Position, Rotation, Scale>(entity);
}

void addTransformComponents(entt::registry& registry, entt::entity entity,
    const glm::vec3& position,
    const glm::vec3& rotationEuler,
    const glm::vec3& scale)
{
    registry.emplace<Position>(entity, position);
    registry.emplace<EulerAngles>(entity, rotationEuler);
    registry.emplace<Rotation>(entity, glm::quat(glm::radians(rotationEuler)));
    registry.emplace<Scale>(entity, scale);
    registry.emplace<ModelMatrix>(entity);
}

GameObject* addGameObjectComponent(entt::registry& registry, entt::entity entity) {
    if (!registry.valid(entity)) {
        return nullptr;
    }

    return &registry.emplace<GameObject>(entity, entity, registry);
}
