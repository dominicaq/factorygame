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

    float elapsedTime = 0.0f;  // Time passed since the object was created
    float lifetime;             // Lifetime in seconds (randomly set)

    void start() override {
        startY = gameObject->getPosition().y;

        // Random X, Y, Z rotation speed between 10 and 100
        rotationSpeed = glm::vec3(
            static_cast<float>(std::rand() % 90 + 10),
            static_cast<float>(std::rand() % 90 + 10),
            static_cast<float>(std::rand() % 90 + 10)
        );

        // Set a random lifetime between 3 and 10 seconds
        lifetime = static_cast<float>(std::rand() % 8 + 3);  // Random lifetime between 3 and 10 seconds
    }

    void update(const float& deltaTime) override {
        elapsedTime += deltaTime;  // Increment the elapsed time by deltaTime

        // Check if lifetime has passed and destroy the object if necessary
        if (elapsedTime >= lifetime) {
            gameObject->destroy();
        }

        glm::vec3 position = gameObject->getPosition();

        // Apply gravity to the object
        velocity -= gravity * deltaTime;  // Apply gravity
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
