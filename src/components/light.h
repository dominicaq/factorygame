#ifndef LIGHT_H
#define LIGHT_H

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include <vector>
#include <limits>

// Max number of lights based on 2-byte IDs (will prob change when I test it)
#define MAX_LIGHTS (std::numeric_limits<unsigned short>::max())

struct LightSystem {
    // Matrices (64 bytes each)
    std::vector<glm::mat4> shadowTransforms;

    // Vectors (12 bytes each)
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> directions;
    std::vector<glm::vec3> colors;

    // Floats (4 bytes each)
    std::vector<float> lightIntensities;
    std::vector<float> radii;

    // Unsigned integers (4 bytes each)
    std::vector<unsigned int> shadowMaps;

    // Booleans (1 byte each)
    std::vector<bool> shadowsEnabled;
    std::vector<bool> directionalFlags;

    // Free indices for removed lights
    std::vector<unsigned short> freeIndices;

    // Current size
    unsigned short size = 0;

    unsigned short addLight(
        const glm::vec3& positionOrDirection,
        const glm::vec3& lightColor = glm::vec3(1.0f),
        float lightRadius = 1.0f,
        float lightIntensity = 2,
        bool shadowEnabled = false,
        bool isDirectional = false,
        const glm::mat4& shadowTransform = glm::mat4(1.0f),
        unsigned int shadowMap = 0)
    {
        if (size >= MAX_LIGHTS) {
            // Handle the case where the maximum number of lights is reached
            return MAX_LIGHTS; // or throw an exception or handle it as needed
        }

        unsigned short index;
        if (!freeIndices.empty()) {
            // Reuse a free index
            index = freeIndices.back();
            freeIndices.pop_back();
        } else {
            // Allocate a new index
            index = size++;
            this->positions.push_back(glm::vec3(0.0f));
            this->directions.push_back(glm::vec3(0.0f));
            this->colors.push_back(glm::vec3(0.0f));
            this->radii.push_back(0.0f);
            this->lightIntensities.push_back(0);
            this->shadowsEnabled.push_back(false);
            this->shadowTransforms.push_back(glm::mat4(1.0f));
            this->shadowMaps.push_back(0);
            this->directionalFlags.push_back(false);
        }

        // Update the light data
        glm::vec3 position = isDirectional ? glm::vec3(0.0f) : positionOrDirection;
        glm::vec3 direction = isDirectional ? positionOrDirection : glm::vec3(0.0f);
        float radius = isDirectional ? 0.0f : lightRadius;

        this->positions[index] = position;
        this->directions[index] = direction;
        this->colors[index] = lightColor;
        this->radii[index] = radius;
        this->lightIntensities[index] = lightIntensity;
        this->shadowsEnabled[index] = shadowEnabled;
        this->shadowTransforms[index] = shadowTransform;
        this->shadowMaps[index] = shadowMap;
        this->directionalFlags[index] = isDirectional;

        return index;
    }

    void removeLight(unsigned short targetID) {
        if (targetID < size) {
            // Mark this index as free
            freeIndices.push_back(targetID);
        }
    }
};

#endif // LIGHT_H
