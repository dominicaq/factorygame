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
    void loadScene(ECSWorld& world, LightSystem& lightSystem, std::vector<GameObject*>& gameObjects) {
        // ------------------------ Setup Camera ------------------------
        Camera camera;
        camera.position = glm::vec3(4.0f, 0.21f, 4.04f);
        camera.eulerAngles = glm::vec3(-2.38f, 239.0f, 0.0f);
        camera.setNearPlane(0.1f);
        camera.setFarPlane(100.0f);
        camera.setFov(50.0f);
        world.insertResource<Camera>(camera);

        // ------------------------ Shader Setup ------------------------

        std::string vertexPath = SHADER_DIR + "default.vs";
        std::string fragmentPath = SHADER_DIR + "default.fs";
        Shader* basicShader = new Shader(vertexPath, fragmentPath);

        // --------------------- Stanford Bunny Model ---------------------
        Entity bunnyEntity = world.createEntity();
        Transform bunnyTransform(glm::vec3(0.0f, -0.5f, 0.0f), glm::vec3(5.0f), glm::vec3(0.0f));
        world.addComponent<Transform>(bunnyEntity, bunnyTransform);
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
        world.addComponent<Transform>(diabloEntity, diabloTransform);
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
        Transform cubeTransform(glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(0.5f), glm::vec3(0.0f));
        world.addComponent<Transform>(cubeEntity, cubeTransform);
        world.addComponent<ModelMatrix>(cubeEntity);

        Mesh* cubeMesh = ResourceLoader::loadMesh(MODEL_DIR + "cube.obj");
        if (cubeMesh != nullptr) {
            Material* cubeMaterial = new Material(forwardShader);
            cubeMaterial->albedoColor = glm::vec3(0.2f, 0.7f, 0.2f);  // Greenish color
            cubeMaterial->isDeferred = false;

            cubeMesh->material = cubeMaterial;
            world.addComponent<Mesh>(cubeEntity, cubeMesh);

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
}
#endif // SCENE_H