#ifndef LIGHT_H
#define LIGHT_H

#include "engine.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <limits>

#define MAX_LIGHTS 100

enum class LightType {
    Point,
    Spotlight,
    Directional
};

struct Light {
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 1.0f;
    float radius = 1.0f;
    LightType type = LightType::Point;
    bool castsShadows = false;
    bool isActive = true;
};

struct Shadow {
    glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);
    unsigned int shadowMapTexture = 0;
    bool dirty = false;
};

#endif // LIGHT_H
