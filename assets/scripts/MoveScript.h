#ifndef MOVESCRIPT_H
#define MOVESCRIPT_H

#include "engine.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include <iostream>

class MoveScript : public Script {
public:
    // Public Script Properties
    glm::vec3 rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);  // Axis of rotation
    float rotationSpeed = 50.0f;  // Rotation speed in degrees per second

private:
    glm::vec3 rotation;

public:
    void start() override {
        std::cout << "MoveScript started for entity: " << gameObject->getEntity().id << "\n";
        // Initialize the rotation from the GameObject's current rotation
        rotation = gameObject->getRotation();
    }

    void update(float deltaTime) override {
        if (!gameObject) {
            std::cout << "MoveScript: No GameObject for entity: " << gameObject->getEntity().id << "\n";
            return;
        }

        // Calculate the rotation amount in radians
        float rotationAmount = glm::radians(rotationSpeed) * deltaTime;

        // Update the object's rotation around the specified axis
        rotation += rotationAxis * rotationAmount;  // Apply rotation in radians
        gameObject->setRotation(rotation);  // Update the rotation using GameObject's setter
    }
};

#endif // MOVESCRIPT_H
