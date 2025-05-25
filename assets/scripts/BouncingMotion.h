#pragma once

#include "engine.h"
#include <glm/glm.hpp>
#include <cstdlib>
#include <cmath>

class BouncingMotion final : public Script {
public:
    float borderSize = 25.0f;
    float movementSpeed = 8.0f;
    float bounceDamping = 0.85f; // Energy loss on bounce (0.0 = no bounce, 1.0 = perfect bounce)
    float gravity = -9.8f;
    float groundLevel = 0.0f;
    float groundBounceDamping = 0.7f; // Separate damping for ground bounces
    float highBounceChance = 0.3f; // 30% chance for high bounce
    float lowBounceChance = 0.3f; // 30% chance for low bounce (40% normal)

private:
    glm::vec3 velocity;
    float timeAlive = 0.0f;
    bool hasInitialVelocity = false;

    void start() override {
        // Set initial position above ground
        glm::vec3 startPos = gameObject->getPosition();
        startPos.y = 5.0f + (std::rand() % 10); // Random height between 5-15
        gameObject->setPosition(startPos);

        // Set random initial velocity
        velocity = glm::vec3(
            static_cast<float>(std::rand() % 200 - 100) / 100.0f * movementSpeed,
            static_cast<float>(std::rand() % 100 + 50) / 100.0f * movementSpeed, // Upward velocity
            static_cast<float>(std::rand() % 200 - 100) / 100.0f * movementSpeed
        );
        hasInitialVelocity = true;
    }

    void update(const float& deltaTime) override {
        timeAlive += deltaTime;

        glm::vec3 position = gameObject->getPosition();

        // Apply gravity to Y velocity
        velocity.y += gravity * deltaTime;

        // Update position based on velocity
        position += velocity * deltaTime;

        // Ground collision (Y-axis)
        if (position.y <= groundLevel) {
            position.y = groundLevel;
            if (velocity.y < 0) { // Only bounce if moving downward
                // Random bounce height variation
                float bounceRandom = static_cast<float>(std::rand()) / RAND_MAX;
                float bounceMultiplier;

                if (bounceRandom < highBounceChance) {
                    // High bounce: 1.5x to 3x energy
                    bounceMultiplier = 1.5f + (static_cast<float>(std::rand()) / RAND_MAX) * 1.5f;
                } else if (bounceRandom < highBounceChance + lowBounceChance) {
                    // Low bounce: 0.2x to 0.5x energy
                    bounceMultiplier = 0.2f + (static_cast<float>(std::rand()) / RAND_MAX) * 0.3f;
                } else {
                    // Normal bounce
                    bounceMultiplier = 1.0f;
                }

                velocity.y = -velocity.y * groundBounceDamping * bounceMultiplier;

                // Random chance to add extra energy/impulse on ground bounce
                if (static_cast<float>(std::rand()) / RAND_MAX < 0.15f) { // 15% chance
                    addRandomImpulse();
                }

                // Stop tiny bounces to prevent jittering (but only for very low bounces)
                if (std::abs(velocity.y) < 0.3f && bounceMultiplier < 0.6f) {
                    velocity.y = 0.0f;
                }
            }
        }

        // Wall collisions (X and Z axes)
        checkAndBounceBorders(position);

        // Random impulse chance every frame (very small probability)
        if (static_cast<float>(std::rand()) / RAND_MAX < 0.001f) { // 0.1% chance per frame
            addRandomImpulse();
        }

        // Random re-energize for balls that are getting too slow
        if (glm::length(velocity) < 1.0f && static_cast<float>(std::rand()) / RAND_MAX < 0.005f) { // 0.5% chance
            addRandomImpulse();
        }

        // Apply some air resistance to prevent infinite motion
        velocity *= 0.999f;

        gameObject->setPosition(position);
    }

    void checkAndBounceBorders(glm::vec3& position) {
        // Right wall
        if (position.x > borderSize) {
            position.x = borderSize;
            if (velocity.x > 0) {
                velocity.x = -velocity.x * bounceDamping;
                addRandomBounceVariation();
            }
        }
        // Left wall
        if (position.x < -borderSize) {
            position.x = -borderSize;
            if (velocity.x < 0) {
                velocity.x = -velocity.x * bounceDamping;
                addRandomBounceVariation();
            }
        }
        // Front wall
        if (position.z > borderSize) {
            position.z = borderSize;
            if (velocity.z > 0) {
                velocity.z = -velocity.z * bounceDamping;
                addRandomBounceVariation();
            }
        }
        // Back wall
        if (position.z < -borderSize) {
            position.z = -borderSize;
            if (velocity.z < 0) {
                velocity.z = -velocity.z * bounceDamping;
                addRandomBounceVariation();
            }
        }
    }

    // Add slight random variation to bounces to prevent predictable patterns
    void addRandomBounceVariation() {
        float variation = 0.1f; // Small variation amount
        velocity.x += (static_cast<float>(std::rand()) / RAND_MAX - 0.5f) * variation * movementSpeed;
        velocity.z += (static_cast<float>(std::rand()) / RAND_MAX - 0.5f) * variation * movementSpeed;
    }

    // Add random impulse to re-energize balls
    void addRandomImpulse() {
        float impulseStrength = movementSpeed * 0.5f; // Half of movement speed

        // Random chance for super high impulse
        if (static_cast<float>(std::rand()) / RAND_MAX < 0.2f) { // 20% chance
            impulseStrength *= 2.5f; // Make it much stronger
        }

        velocity += glm::vec3(
            (static_cast<float>(std::rand()) / RAND_MAX - 0.5f) * impulseStrength,
            std::abs(static_cast<float>(std::rand()) / RAND_MAX) * impulseStrength, // Always upward
            (static_cast<float>(std::rand()) / RAND_MAX - 0.5f) * impulseStrength
        );
    }
};
