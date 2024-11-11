#pragma once

#include "engine.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

class MoveScript : public Script {
public:
    // Public Script Properties
    glm::vec3 rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
    float rotationSpeed = 50.0f;

private:
    glm::vec3 m_eulerRotation;

public:
    void start() override {
        std::cout << "MoveScript started for entity: " << gameObject->getName() << "\n";
        m_eulerRotation = gameObject->getEuler();
    }

    void update(float deltaTime) override {
        if (!gameObject) {
            std::cout << "MoveScript: No GameObject for entity: " << "\n";
            return;
        }

        // Calculate the rotation amount in degrees
        float rotationAmount = rotationSpeed * deltaTime;

        // Update the object's Euler rotation around the specified axis
        m_eulerRotation += rotationAxis * rotationAmount;
        gameObject->setEuler(m_eulerRotation);
    }
};
