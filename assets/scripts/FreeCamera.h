#ifndef FREE_CAMERA_H
#define FREE_CAMERA_H

#include "engine.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

class FreeCamera : public Script {
public:
    float cameraSpeed = 2.5f;
    float sensitivity = 0.1f;

    void start() override {
        inputManager.setCursorMode(GLFW_CURSOR_DISABLED);
    }

    void update(float deltaTime) override {
        // Get the camera's position and Euler angles from the GameObject
        glm::vec3 position = gameObject->getPosition();
        glm::vec3 eulerAngles = gameObject->getEuler();  // Use Euler angles instead of quaternions

        // Calculate camera front vector based on Euler angles
        glm::vec3 front;
        front.x = cos(glm::radians(eulerAngles.y)) * cos(glm::radians(eulerAngles.x));
        front.y = sin(glm::radians(eulerAngles.x));
        front.z = sin(glm::radians(eulerAngles.y)) * cos(glm::radians(eulerAngles.x));
        front = glm::normalize(front);

        glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
        glm::vec3 up = glm::normalize(glm::cross(right, front));

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

#endif // FREE_CAMERA_H
