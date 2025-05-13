#pragma once

#include "engine.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <cstdlib>
#include <cmath>

class ScaleScript : public Script {
public:
    // Public Script Properties
    glm::vec3 rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
    float rotationSpeed = 50.0f;
    float scaleAmplitude = 0.75f; // Max deviation from initial scale
    float scaleFrequency = 0.5f;  // Oscillations per second

private:
    glm::vec3 m_eulerRotation;
    glm::vec3 m_initialScale;
    float m_elaspedTime = 0.0f;

public:
    void start() override {
        m_eulerRotation = gameObject->getEuler();
        m_initialScale = gameObject->getScale();
    }

    void update(const float& deltaTime) override {
        if (!gameObject) {
            std::cout << "MoveScript: No GameObject for entity.\n";
            return;
        }

        m_elaspedTime += deltaTime;

        // --- Rotation Logic ---
        float rotationAmount = rotationSpeed * deltaTime;
        m_eulerRotation += rotationAxis * rotationAmount;
        gameObject->setEuler(m_eulerRotation);

        // --- Scale Pulsing Logic ---
        float scaleOffset = std::sin(m_elaspedTime * scaleFrequency * 2.0f * glm::pi<float>()) * scaleAmplitude;
        glm::vec3 currentScale = m_initialScale + glm::vec3(scaleOffset);
        gameObject->setScale(currentScale);
    }
};
