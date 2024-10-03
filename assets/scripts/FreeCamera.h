#ifndef FREE_CAMERA_H
#define FREE_CAMERA_H

#include "engine.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include <iostream>

class FreeCamera : public Script {
public:
    float cameraSpeed = 2.5f;
    float sensitivity = 0.1f;

    void start() override {
        inputManager.setCursorMode(GLFW_CURSOR_DISABLED);
    }

    void update(float deltaTime) override {
        // Get the camera's position and rotation components from the ECS
        auto positionComponent = gameObject->getComponent<PositionComponent>();
        auto rotationComponent = gameObject->getComponent<RotationComponent>();

        // Calculate camera front vector based on Euler angles
        glm::vec3 front;
        front.x = cos(glm::radians(rotationComponent->eulerAngles.y)) * cos(glm::radians(rotationComponent->eulerAngles.x));
        front.y = sin(glm::radians(rotationComponent->eulerAngles.x));
        front.z = sin(glm::radians(rotationComponent->eulerAngles.y)) * cos(glm::radians(rotationComponent->eulerAngles.x));
        front = glm::normalize(front);

        glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
        glm::vec3 up = glm::normalize(glm::cross(right, front));

        float dt = cameraSpeed * deltaTime;

        // Handle movement
        if (inputManager.isKeyPressed(GLFW_KEY_W)) {
            positionComponent->position += dt * front;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_S)) {
            positionComponent->position -= dt * front;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_A)) {
            positionComponent->position -= right * dt;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_D)) {
            positionComponent->position += right * dt;
        }

        // Mouse input for look rotation
        if (inputManager.isCursorDisabled()) {
            float xOffset = inputManager.getMouseXOffset() * sensitivity;
            float yOffset = inputManager.getMouseYOffset() * sensitivity;

            // Adjust camera angles
            rotationComponent->eulerAngles.y += xOffset;
            rotationComponent->eulerAngles.x += yOffset;

            // Constrain the pitch to avoid gimbal lock
            if (rotationComponent->eulerAngles.x > 89.0f) {
                rotationComponent->eulerAngles.x = 89.0f;
            }
            if (rotationComponent->eulerAngles.x < -89.0f) {
                rotationComponent->eulerAngles.x = -89.0f;
            }
        }
    }
};

#endif // FREE_CAMERA_H
