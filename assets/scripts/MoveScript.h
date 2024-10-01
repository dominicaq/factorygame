#ifndef MOVESCRIPT_H
#define MOVESCRIPT_H

#include "engine.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include <iostream>

class MoveScript : public Script {
public:
    // Reference to component(s)
    Transform* transform = nullptr;

    // Script Properties
    glm::vec3 rotation;
    glm::vec3 rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
    float rotationSpeed = 50.0f;

    void start() override {
        std::cout << "MoveScript started for entity: " << gameObject->getEntity().id << "\n";
        transform = gameObject->getComponent<Transform>();
        rotation = transform->getEulerAngles();
    }

    void update(float deltaTime) override {
        if (transform == nullptr) {
            std::cout << "MoveScript no transform from entity: : " << gameObject->getEntity().id << "\n";
            return;
        }

        // Calculate the rotation in degrees
        float rotationAmount = rotationSpeed * deltaTime;

        // Update the object's eulerAngles around the specified axis
        rotation += rotationAxis * rotationAmount;
        transform->setRotation(rotation);
    }
};

#endif // MOVESCRIPT_H
