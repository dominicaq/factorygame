#ifndef MOVESCRIPT_H
#define MOVESCRIPT_H

#include "engine.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include <iostream>

class MoveScript : public Script {
public:
    glm::vec3 rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f); // Rotate around the Y-axis by default
    float rotationSpeed = 50.0f; // Degrees per second
    glm::vec3 rot = glm::vec3(1.0f);

    void start() override {
        std::cout << "MoveScript started for entity: " << gameObject->getEntity().id << "\n";
        rot = gameObject->getComponent<Transform>()->getEulerAngles();
    }

    void update(float deltaTime) override {
        // Get the transform component of the associated entity
        Transform& transform = *gameObject->getComponent<Transform>();

        // Calculate the rotation in degrees
        float rotationAmount = rotationSpeed * deltaTime;

        // Update the object's eulerAngles around the specified axis
        rot += rotationAxis * rotationAmount;
        transform.setRotation(rot);
    }
};

#endif // MOVESCRIPT_H
