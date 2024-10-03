#ifndef SCENE_H
#define SCENE_H

#include "engine.h"

// Scripts
#include "MoveScript.h"
#include "ViewFramebuffers.h"
#include "FreeCamera.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// Asset paths
static const std::string SHADER_DIR = ASSET_DIR "shaders/";
static const std::string MODEL_DIR = ASSET_DIR "models/";
static const std::string TEXTURE_DIR = ASSET_DIR "textures/";

namespace Scene {
    void loadScene(ECSWorld& world, LightSystem& lightSystem, GameObjectManager& gameObjectManager) {
        // ------------------------ Setup Camera ------------------------
        // Add PositionComponent and RotationComponent to the camera entity
        Entity cameraEntity = world.createEntity();
        world.addComponent<PositionComponent>(cameraEntity, { glm::vec3(4.0f, 0.21f, 4.04f) });
        world.addComponent<RotationComponent>(cameraEntity, { glm::vec3(-2.38f, 239.0f, 0.0f) });

        // Create a Camera object and set its projection properties
        Camera camera(cameraEntity, &world);
        camera.setNearPlane(0.1f);
        camera.setFarPlane(100.0f);
        camera.setFov(50.0f);
        camera.setAspectRatio(SCREEN_WIDTH, SCREEN_HEIGHT);

        // Insert the camera as a resource in the ECS world
        world.insertResource<Camera>(camera);

        // Use GameObjectManager to create and manage the GameObject for the camera
        GameObject* cameraObject = gameObjectManager.createGameObject(cameraEntity);
        cameraObject->addScript<FreeCamera>();

        // ------------------------ Shader Setup ------------------------
        std::string vertexPath = SHADER_DIR + "default.vs";
        std::string fragmentPath = SHADER_DIR + "default.fs";
        Shader* basicShader = new Shader(vertexPath, fragmentPath);

        // --------------------- Stanford Bunny Model ---------------------
        Entity bunnyEntity = world.createEntity();
        world.addComponent<PositionComponent>(bunnyEntity, { glm::vec3(0.0f, -0.5f, 0.0f) });
        world.addComponent<RotationComponent>(bunnyEntity, { glm::vec3(0.0f) });
        world.addComponent<ScaleComponent>(bunnyEntity, { glm::vec3(5.0f) });
        world.addComponent<ModelMatrix>(bunnyEntity);

        Mesh* bunnyMesh = ResourceLoader::loadMesh(MODEL_DIR + "stanfordBunny.obj");
        if (bunnyMesh != nullptr) {
            Material* bunnyMaterial = new Material(basicShader);
            bunnyMaterial->albedoColor = glm::vec3(1.0f, 0.5f, 0.31f);
            bunnyMaterial->isDeferred = true;

            Texture* bunnyAlbedoMap = new Texture(TEXTURE_DIR + "uv_map.jpg");
            bunnyMaterial->albedoMap = bunnyAlbedoMap;

            bunnyMesh->material = bunnyMaterial;
            world.addComponent<Mesh>(bunnyEntity, bunnyMesh);

            // Use GameObjectManager to create and manage the GameObject
            GameObject* bunnyObject = gameObjectManager.createGameObject(bunnyEntity);
            bunnyObject->addScript<MoveScript>();
        }

        // --------------------- Dummy Entity (global scripts) ------------------
        Entity dummyEntity = world.createEntity();
        GameObject* dummyObject = gameObjectManager.createGameObject(dummyEntity);
        dummyObject->addScript<ViewFrameBuffers>();

        // --------------------- Diablo Model ---------------------
        Entity diabloEntity = world.createEntity();
        world.addComponent<PositionComponent>(diabloEntity, { glm::vec3(2.0f, 0.0f, -1.0f) });
        world.addComponent<RotationComponent>(diabloEntity, { glm::vec3(0.0f) });
        world.addComponent<ScaleComponent>(diabloEntity, { glm::vec3(2.0f) });
        world.addComponent<ModelMatrix>(diabloEntity);

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
            world.addComponent<Mesh>(diabloEntity, diabloModel);
        }

        // --------------------- Cube Model (Forward Rendering) ---------------------
        std::string forwardVertexPath = SHADER_DIR + "default.vs";
        std::string forwardFragmentPath = SHADER_DIR + "default.fs";
        Shader* forwardShader = new Shader(forwardVertexPath, forwardFragmentPath);

        Entity cubeEntity = world.createEntity();
        world.addComponent<PositionComponent>(cubeEntity, { glm::vec3(0.0f, 2.0f, 0.0f) });
        world.addComponent<RotationComponent>(cubeEntity, { glm::vec3(0.0f) });
        world.addComponent<ScaleComponent>(cubeEntity, { glm::vec3(0.5f) });
        world.addComponent<ModelMatrix>(cubeEntity);

        Mesh* cubeMesh = ResourceLoader::loadMesh(MODEL_DIR + "cube.obj");
        if (cubeMesh != nullptr) {
            Material* cubeMaterial = new Material(forwardShader);
            cubeMaterial->albedoColor = glm::vec3(0.2f, 0.7f, 0.2f);  // Greenish color
            cubeMaterial->isDeferred = false;

            cubeMesh->material = cubeMaterial;
            world.addComponent<Mesh>(cubeEntity, cubeMesh);

            // Use GameObjectManager to create and manage the GameObject
            GameObject* cubeObject = gameObjectManager.createGameObject(cubeEntity);
            cubeObject->addScript<MoveScript>();
        }

        // --------------------- Light System ---------------------
        lightSystem.addLight(
            glm::vec3(-20.0f, 0.0f, 0.0f),  // Middle-left side
            glm::vec3(0.2f, 0.2f, 0.2f),    // Grayish White color
            1.0f,                          // Light radius
            3.0f,                           // Light intensity
            false,                          // No shadows
            false                           // Point light
        );

        lightSystem.addLight(
            glm::vec3(-2.0f, 2.5f, 2.0f),  // Position for a point light
            glm::vec3(0.0f, 0.0f, 0.5f),   // Blue light color
            10.0f,                         // Light radius
            1.0f,                          // Light intensity
            false,                         // No shadows
            false                          // Point light
        );
    }
}

#endif // SCENE_H
