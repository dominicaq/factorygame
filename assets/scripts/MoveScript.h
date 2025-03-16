#pragma once

#include "engine.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <cstdlib>

class MoveScript : public Script {
public:
    // Public Script Properties
    glm::vec3 rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
    float rotationSpeed = 50.0f;

private:
    glm::vec3 m_eulerRotation;
    float elapsedTime = 0.0f;  // Track elapsed time
    float lifetime;             // Random lifetime

public:
    void start() override {
        std::cout << "MoveScript started for entity: " << gameObject->getName() << "\n";
        m_eulerRotation = gameObject->getEuler();

        // Set a random lifetime between 3 and 10 seconds
        lifetime = static_cast<float>(std::rand() % 8 + 3);
    }

    void update(const float& deltaTime) override {
        if (!gameObject) {
            std::cout << "MoveScript: No GameObject for entity.\n";
            return;
        }

        elapsedTime += deltaTime;  // Increment elapsed time

        // Check if lifetime has passed and destroy the object if necessary
        if (elapsedTime >= lifetime) {
            gameObject->destroy();
            return;
        }

        // Calculate the rotation amount in degrees
        float rotationAmount = rotationSpeed * deltaTime;

        // Update the object's Euler rotation around the specified axis
        m_eulerRotation += rotationAxis * rotationAmount;
        gameObject->setEuler(m_eulerRotation);
    }
};
