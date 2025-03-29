#pragma once

#include "engine.h"
#include <glm/glm.hpp>
#include <cstdlib>
#include <cmath>

class BouncingMotion final : public Script {
public:
    float borderSize = 40.0f;
    float movementSpeed = 5.0f;
    float directionChangeDuration = 3.0f; // More frequent direction shifts

private:
    glm::vec3 velocityDirection;
    glm::vec3 targetDirection;
    float timeSinceDirectionChange = 0.0f;
    float speedMultiplier = 1.0f; // Oscillating speed effect

    void start() override {
        setRandomDirection();
        targetDirection = velocityDirection;
        gameObject->setPosition(glm::vec3(gameObject->getPosition().x, 0, gameObject->getPosition().z));
    }

    void update(const float& deltaTime) override {
        timeSinceDirectionChange += deltaTime;

        // Smoothly interpolate between directions
        if (timeSinceDirectionChange >= directionChangeDuration) {
            setRandomDirection();
            timeSinceDirectionChange = 0.0f;
        }
        velocityDirection = glm::mix(velocityDirection, targetDirection, deltaTime * 2.0f);

        // Oscillate speed for variety
        speedMultiplier = 0.8f + 0.4f * std::sin(timeSinceDirectionChange * 2.0f);

        glm::vec3 position = gameObject->getPosition();
        position += velocityDirection * deltaTime * movementSpeed * speedMultiplier;

        checkAndTeleportBorders(position);
        gameObject->setPosition(glm::vec3(position.x, 0, position.z));
    }

    void setRandomDirection() {
        targetDirection = glm::normalize(glm::vec3(
            static_cast<float>(std::rand() % 200 - 100) / 100.0f,
            0.0f,
            static_cast<float>(std::rand() % 200 - 100) / 100.0f
        ));
    }

    void checkAndTeleportBorders(glm::vec3& position) {
        if (position.x > borderSize) position.x = -borderSize;
        if (position.x < -borderSize) position.x = borderSize;
        if (position.z > borderSize) position.z = -borderSize;
        if (position.z < -borderSize) position.z = borderSize;
    }
};
