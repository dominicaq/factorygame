#pragma once

#include "engine.h"
#include <glm/glm.hpp>
#include <cstdlib>
#include <cmath>

class BouncingMotion final : public Script {
public:
    float fallSpeed = 2.0f;
    float riseSpeed = 3.0f;

private:
    float startY;
    float velocity = 0.0f;
    float gravity = 9.8f;
    glm::vec3 rotationSpeed;

    void start() override {
        startY = gameObject->getPosition().y;

        // Random X, Y, Z rotation speed between 10 and 100
        rotationSpeed = glm::vec3(
            static_cast<float>(std::rand() % 90 + 10),
            static_cast<float>(std::rand() % 90 + 10),
            static_cast<float>(std::rand() % 90 + 10)
        );
    }

    void update(const float& deltaTime) override {
        glm::vec3 position = gameObject->getPosition();

        // Apply gravity to the object
        velocity -= gravity * deltaTime; // Apply gravity
        position.y += velocity * deltaTime;

        // Check if the object has hit the ground (y = 0.0f)
        if (position.y <= -0.5f) {
            position.y = -0.5f;  // Make sure it doesn't go below the ground
            velocity = std::sqrt(2 * gravity * (startY));
        }

        // Apply the position update
        gameObject->setPosition(position);

        // Apply rotation on all axes
        glm::vec3 rotation = gameObject->getEuler();
        rotation += rotationSpeed * deltaTime;
        gameObject->setEuler(rotation);
    }
};
