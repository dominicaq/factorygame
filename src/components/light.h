#ifndef LIGHT_H
#define LIGHT_H

#include "engine.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define MAX_LIGHTS 1000
#define MAX_SHADOW_MAPS 20

enum class LightType : uint8_t {
    Point,
    Spot,
    Directional
};

struct Light {
    glm::vec3 color;
    float intensity;

    glm::vec3 position;
    uint32_t depthHandle;

    glm::vec3 direction;
    LightType type;
    bool castsShadows;
    bool isActive;
    uint8_t _padding;

    union {
        struct {
            float radius;
            float _pointPadding;
        } point;

        struct {
            float innerCutoff;
            float outerCutoff;
        } spot;

        struct {
            float shadowOrthoSize;
            float _dirPadding;
        } directional;
    };
};


struct LightSpaceMatrix {
    glm::mat4 matrix = glm::mat4(1.0f);
};

struct LightSpaceMatrixCube {
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
