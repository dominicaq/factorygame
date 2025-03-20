#pragma once

#include "engine.h"
#include <glm/glm.hpp>
#include <cstdlib>
#include <cmath>

class BouncingMotion final : public Script {
public:
    float borderSize = 40.0f;  // Set the border size as a parameter
    float movementSpeed = 5.0f; // Speed at which the object moves
    float directionChangeDuration = 5.0f;  // Duration to smoothly change direction (in seconds)

private:
    glm::vec3 velocityDirection;  // Current direction of the object (only X and Z components)
    glm::vec3 targetDirection;    // Target direction after change (only X and Z components)
    float timeSinceDirectionChange = 0.0f; // Time elapsed since last direction change

    void start() override {
        // Set initial random direction (X and Z only, Y is fixed)
        setRandomDirection();
        targetDirection = velocityDirection; // Initially, the target direction is the same as the starting direction
        glm::vec3 startPos = gameObject->getPosition();
        startPos.y = 0;
        gameObject->setPosition(startPos);
    }

    void update(const float& deltaTime) override {
        glm::vec3 position = gameObject->getPosition();

        // Move the object in the current direction (X and Z only)
        position.x += velocityDirection.x * deltaTime * movementSpeed;
        position.z += velocityDirection.z * deltaTime * movementSpeed;

        // Make the object face the direction it's moving towards
        glm::vec3 forwardDirection = velocityDirection;
        glm::vec3 objectToTarget = glm::normalize(forwardDirection);

        // Calculate the rotation to face the target direction
        glm::vec3 rotation = gameObject->getEuler();
        rotation.x = std::atan2(objectToTarget.z, objectToTarget.y); // Update rotation on the X axis
        rotation.y = std::atan2(objectToTarget.x, objectToTarget.z); // Update rotation on the Y axis
        gameObject->setEuler(rotation);

        // Check for borders and teleport if needed
        checkAndTeleportBorders(position);

        // Apply the position update (preserve the Y position)
        gameObject->setPosition(glm::vec3(position.x, 0, position.z));
    }

    void setRandomDirection() {
        // Set a random direction for the velocity (only X and Z components)
        velocityDirection = glm::vec3(
            static_cast<float>(std::rand() % 200 - 100) / 100.0f, // Random X direction (-1 to 1)
            0.0f, // Y is fixed
            static_cast<float>(std::rand() % 200 - 100) / 100.0f  // Random Z direction (-1 to 1)
        );
        velocityDirection = glm::normalize(velocityDirection);  // Normalize direction to keep speed consistent
    }

    void checkAndTeleportBorders(glm::vec3& position) {
        // Teleport the object to the opposite side if it goes beyond the borders in the X or Z axis
        if (position.x > borderSize) position.x = -borderSize; // Teleport to the left side in the X axis
        if (position.x < -borderSize) position.x = borderSize; // Teleport to the right side in the X axis
        if (position.z > borderSize) position.z = -borderSize; // Teleport to the front side in the Z axis
        if (position.z < -borderSize) position.z = borderSize; // Teleport to the back side in the Z axis
    }
};
