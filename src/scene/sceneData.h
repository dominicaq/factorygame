#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

// Serialized data saved to disk
struct SceneData {
    std::string name;

    // Transform
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    glm::vec3 eulerAngles = glm::vec3(0.0f);

    std::vector<size_t> children;
};
