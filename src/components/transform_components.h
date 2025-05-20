#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <vector>
#include <entt/entt.hpp>
#include <bitset>

struct EntityStatus {
    enum FLAG : std::size_t {
        DIRTY_MODEL_MATRIX,
        DESTROY_ENTITY,
        // Add more flags ...
        FLAG_COUNT
    };

    std::bitset<FLAG_COUNT> status;
};

struct ModelMatrix {
    glm::mat4 matrix = glm::mat4(1.0f);
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
