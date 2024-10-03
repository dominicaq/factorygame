#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "glm.hpp"

struct PositionComponent {
    glm::vec3 position = glm::vec3(0.0f);
};

struct RotationComponent {
    glm::vec3 eulerAngles = glm::vec3(0.0f);
};

struct ScaleComponent {
    glm::vec3 scale = glm::vec3(1.0f);
};

#endif // TRANSFORM_H
