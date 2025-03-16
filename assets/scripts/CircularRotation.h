#pragma once

#include "engine.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class CircularRotation : public Script {
public:
    float rotationSpeed = 1.0f;
    float radius = 5.0f;
    glm::vec3 center = glm::vec3(0.0f);

private:
    float m_angle = 0.0f;

    void start() override {
        glm::vec3 startPosition = gameObject->getPosition();
        glm::vec3 offset = startPosition - center;
        m_angle = std::atan2(offset.z, offset.x);
        gameObject->setPosition(startPosition);
    }

    void update(const float& deltaTime) override {
        m_angle += rotationSpeed * deltaTime;
        if (m_angle > glm::two_pi<float>()) {
            m_angle -= glm::two_pi<float>();
        }

        glm::vec3 newPosition;
        newPosition.x = center.x + radius * cos(m_angle);
        newPosition.z = center.z + radius * sin(m_angle);
        newPosition.y = gameObject->getPosition().y;
        gameObject->setPosition(newPosition);
    }
};
