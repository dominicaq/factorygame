#ifndef LIGHT_H
#define LIGHT_H

#include "engine.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include <vector>
#include <limits>

// enum class LightType {
//     Point,
//     Spotlight,
//     Directional
// };

// struct Light {
//     glm::vec3 color;
//     float intensity;
//     union {
//         float range;
//         struct {
//             float spotAngle;
//             float outerSpotAngle;
//         };
//     };
//     float attenuation[3];
//     bool castsShadows;
//     bool isActive;
//     LightType type;
//     uint8_t padding[1];
//     unsigned int shadowMapTexture;
// };

// struct LightSpaceMatrix {
//     glm::mat4 matrix;
//     bool dirty = false;
// };

// #define MAX_LIGHTS 100
// static int NUM_LIGHTS = 0;

// class LightSystems {
// public:
//     inline void updateLights(ECSWorld& world) {
//         // Only query if it changed
//         if (!m_validCache) {
//             m_entityCache = world.batchedQuery<Light>();
//             m_validCache = true;
//         }

//         for (Entity entity : m_entityCache) {

//         }
//     }

// private:
//     ECSWorld& m_world;
//     std::vector<Entity> m_entityCache;
//     bool m_validCache = false;
// };

#define MAX_LIGHTS 100

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
            return MAX_LIGHTS;
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
