#ifndef TRANSFORM_COMPONENTS_H
#define TRANSFORM_COMPONENTS_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <vector>
#include <entt/entt.hpp>

struct ModelMatrix {
    glm::mat4 matrix = glm::mat4(1.0f);
    bool dirty = true;
};

struct Position {
    glm::vec3 position = glm::vec3(0.0f);
};

struct Rotation {
    glm::quat quaternion = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
};

struct EulerAngles {
    glm::vec3 euler = glm::vec3(0.0f);
};

struct Scale {
    glm::vec3 scale = glm::vec3(1.0f);
};

struct Parent {
    entt::entity parent;
};

struct Children {
    std::vector<entt::entity> children;
};

#endif // TRANSFORM_COMPONENTS_H
