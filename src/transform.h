// TEMPORARY IMPLEMENTATION
#pragma once

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

class Transform {
public:
    // Public variables
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    glm::vec3 eulerAngles = glm::vec3(0.0f);

    // Default constructor
    Transform() = default;

    // Parameterized constructor
    Transform(const glm::vec3& pos, const glm::vec3& scl, const glm::vec3& rot)
        : position(pos), scale(scl), eulerAngles(rot) {}

    // Method to get the Model Matrix for the transform
    glm::mat4 getModelMatrix() const {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(eulerAngles.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(eulerAngles.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(eulerAngles.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, scale);
        return model;
    }
};
