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

        // Make the object look at the world origin
        lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
    }

    void lookAt(const glm::vec3& target) {
        glm::vec3 position = gameObject->getPosition();
        glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::mat4 viewMatrix = glm::lookAt(position, target, worldUp);
        glm::mat4 rotationMatrix = glm::inverse(viewMatrix);
        glm::quat rotationQuat = glm::quat_cast(rotationMatrix);
        gameObject->setRotation(rotationQuat);
    }
};
