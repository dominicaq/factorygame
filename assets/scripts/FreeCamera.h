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
        // Get current active camera
        Camera& camera = gameObject->getResource<Camera>();

        // Calculate camera front vector based on Euler angles
        glm::vec3 front;
        front.x = cos(glm::radians(camera.eulerAngles.y)) * cos(glm::radians(camera.eulerAngles.x));
        front.y = sin(glm::radians(camera.eulerAngles.x));
        front.z = sin(glm::radians(camera.eulerAngles.y)) * cos(glm::radians(camera.eulerAngles.x));
        front = glm::normalize(front);

        glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
        glm::vec3 up = glm::normalize(glm::cross(right, front));

        float dt = 2.5f * deltaTime;
        if (inputManager.isKeyPressed(GLFW_KEY_W)) {
            camera.position += dt * front;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_S)) {
            camera.position -= dt * front;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_A)) {
            camera.position -= right * dt;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_D)) {
            camera.position += right * dt;
        }

        // Mouse input for look rotation
        if (inputManager.isCursorDisabled()) {
            float xOffset = inputManager.getMouseXOffset() * sensitivity;
            float yOffset = inputManager.getMouseYOffset() * sensitivity;

            // Adjust camera angles
            camera.eulerAngles.y += xOffset;
            camera.eulerAngles.x += yOffset;

            // Constrain the pitch to avoid gimbal lock
            if (camera.eulerAngles.x > 89.0f) {
                camera.eulerAngles.x = 89.0f;
            }
            if (camera.eulerAngles.x < -89.0f) {
                camera.eulerAngles.x = -89.0f;
            }
        }
    }
};

#endif // FREE_CAMERA_H
