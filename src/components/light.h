#ifndef LIGHT_H
#define LIGHT_H

#include "engine.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define MAX_LIGHTS 1000
#define MAX_SHADOW_MAPS 20

enum class LightType {
    Point,
    Spotlight,
    Directional
};

struct Light {
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 1.0f;
    unsigned int depthHandle = 0;
    float radius = 1.0f;
    LightType type = LightType::Point;
    bool castsShadows = false;
    bool isActive = true;
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
