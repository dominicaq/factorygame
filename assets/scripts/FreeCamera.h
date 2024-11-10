#pragma once

#include "engine.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

class FreeCamera : public Script {
public:
    float defaultSpeed = 2.5f;
    float boostSpeed = 10.0f;
    float sensitivity = 0.1f;

private:
    float cameraSpeed = defaultSpeed;

public:
    void start() override {
        // inputManager.setCursorMode(GLFW_CURSOR_DISABLED);
    }

    void update(float deltaTime) override {
        // Get the camera's position and Euler angles from the GameObject
        glm::vec3 position = gameObject->getPosition();
        glm::vec3 eulerAngles = gameObject->getEuler();

        glm::vec3 front = gameObject->getForward();
        glm::vec3 right = gameObject->getRight();

        float dt = cameraSpeed * deltaTime;

        // Handle movement based on input
        if (inputManager.isKeyPressed(GLFW_KEY_W)) {
            position += dt * front;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_S)) {
            position -= dt * front;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_A)) {
            position -= right * dt;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_D)) {
            position += right * dt;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
            cameraSpeed = boostSpeed;
        } else {
            cameraSpeed = defaultSpeed;
        }

        // Update the camera's position in the GameObject
        gameObject->setPosition(position);

        // Mouse input for look rotation
        if (inputManager.isCursorDisabled()) {
            float xOffset = inputManager.getMouseXOffset() * sensitivity;
            float yOffset = inputManager.getMouseYOffset() * sensitivity;

            // Adjust camera angles
            eulerAngles.y += xOffset;  // Yaw (left/right)
            eulerAngles.x += yOffset;  // Pitch (up/down)

            // Constrain the pitch to avoid gimbal lock
            if (eulerAngles.x > 89.0f) {
                eulerAngles.x = 89.0f;
            }
            if (eulerAngles.x < -89.0f) {
                eulerAngles.x = -89.0f;
            }

            // Update the Euler angles in the GameObject
            gameObject->setEuler(eulerAngles);
        }
    }
};
