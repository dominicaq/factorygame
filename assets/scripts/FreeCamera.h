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
    bool m_flashLightToggle = false;

public:
    void start() override {
        inputManager.setCursorMode(GLFW_CURSOR_DISABLED);
    }

    void update(const float& deltaTime) override {
        // Get the camera's position and Euler angles from the GameObject
        glm::vec3 position = gameObject->getPosition();
        glm::vec3 eulerAngles = gameObject->getEuler();

        glm::vec3 front = gameObject->getForward();
        glm::vec3 right = gameObject->getRight();
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        float dt = cameraSpeed * deltaTime;

        // Lock cursor
        if (inputManager.isKeyPressed(GLFW_KEY_Q)) {
            inputManager.setCursorMode(GLFW_CURSOR_NORMAL);
        }
        if (inputManager.isKeyPressed(GLFW_KEY_E)) {
            inputManager.setCursorMode(GLFW_CURSOR_DISABLED);
        }

        // Handle movement based on input
        if (inputManager.isKeyDown(GLFW_KEY_W)) {
            position += dt * front;
        }
        if (inputManager.isKeyDown(GLFW_KEY_S)) {
            position -= dt * front;
        }
        if (inputManager.isKeyDown(GLFW_KEY_A)) {
            position -= right * dt;
        }
        if (inputManager.isKeyDown(GLFW_KEY_D)) {
            position += right * dt;
        }
        if (inputManager.isKeyDown(GLFW_KEY_SPACE)) {
            position += up * dt * 2.0f;
        }
        if (inputManager.isKeyDown(GLFW_KEY_LEFT_CONTROL)) {
            position -= up * dt * 2.0f;
        }

        // Speed
        if (inputManager.isKeyDown(GLFW_KEY_LEFT_SHIFT)) {
            cameraSpeed = boostSpeed;
        } else {
            cameraSpeed = defaultSpeed;
        }

        // Flash light
        if (inputManager.isKeyPressed(GLFW_KEY_F)) {
            m_flashLightToggle = !m_flashLightToggle;
            auto& light = gameObject->getComponent<Light>();
            light.isActive = m_flashLightToggle;
        }

        // Update the camera's position in the GameObject
        gameObject->setPosition(position);

        // Mouse input for look rotation
        if (inputManager.isCursorDisabled()) {
            float xOffset = -inputManager.getMouseXOffset() * sensitivity;
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
