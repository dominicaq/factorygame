#ifndef LIGHT_H
#define LIGHT_H

#include "engine.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define MAX_LIGHTS 1000
#define MAX_SHADOW_MAPS 20
#define MAX_DIRECTIONAL_LIGHTS 3

enum class LightType : uint8_t {
    Point,
    Spot,
    Directional
};

struct Light {
    // 16-byte alignment (vec3 + uint32)
    uint32_t depthHandle;

    // 16-byte alignment (vec3 + enum)
    glm::vec3 color;
    LightType type;

    // 4-byte alignment
    float intensity;

    // 4-byte alignment (bool + bool + padding)
    bool castShadow;
    bool isActive;
    uint16_t _padding;

    union {
        struct {
            float radius;
        } point;

        struct {
            float innerCutoff;
            float outerCutoff;
            float range;
        } spot;

        struct {
            float shadowOrthoSize;
        } directional;
    };
};

struct LightSpaceMatrix {
    glm::mat4 matrix = glm::mat4(1.0f);
};

struct LightSpaceMatrixArray {
    glm::mat4 matrices[6] = {
        glm::mat4(1.0f), // Right face (+X)
        glm::mat4(1.0f), // Left face (-X)
        glm::mat4(1.0f), // Up face (+Y)
        glm::mat4(1.0f), // Down face (-Y)
        glm::mat4(1.0f), // Front face (+Z)
        glm::mat4(1.0f)  // Back face (-Z)
    };
};

#endif // LIGHT_H
