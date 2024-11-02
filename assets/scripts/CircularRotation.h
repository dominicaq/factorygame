#pragma once

#include "engine.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class CircularRotation : public Script {
public:
    float rotationSpeed = 1.0f;
    float radius = 5.0f;
    glm::vec3 center = glm::vec3(0.0f);
    float angle = 0.0f;

    void start() override {
        glm::vec3 startPosition = gameObject->getPosition();
        glm::vec3 offset = startPosition - center;
        angle = std::atan2(offset.z, offset.x);
        gameObject->setPosition(startPosition);
    }

    void update(float deltaTime) override {
        angle += rotationSpeed * deltaTime;
        if (angle > glm::two_pi<float>()) {
            angle -= glm::two_pi<float>();
        }

        glm::vec3 newPosition;
        newPosition.x = center.x + radius * cos(angle);
        newPosition.z = center.z + radius * sin(angle);
        newPosition.y = gameObject->getPosition().y;
        gameObject->setPosition(newPosition);
    }
};
