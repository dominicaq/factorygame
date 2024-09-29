#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

// Define PI and DEG_TO_RAD using macros
#define PI 3.14159265358979323846f
#define DEG_TO_RAD (PI / 180.0f)

class Transform {
public:
    // Default constructor
    Transform() : m_position(0.0f), m_scale(1.0f), m_eulerAngles(0.0f), m_isDirty(true) {}

    // Parameterized constructor
    Transform(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& eulerAngles)
        : m_position(position), m_scale(scale), m_eulerAngles(eulerAngles), m_isDirty(true) {}

    const glm::vec3& getPosition() const {
        return m_position;
    }

    void setPosition(const glm::vec3& position) {
        m_position = position;
        m_isDirty = true;
    }

    const glm::vec3& getScale() const {
        return m_scale;
    }

    void setScale(const glm::vec3& scale) {
        m_scale = scale;
        m_isDirty = true;
    }

    const glm::vec3& getEulerAngles() const {
        return m_eulerAngles;
    }

    void setRotation(const glm::vec3& eulerAngles) {
        m_eulerAngles = eulerAngles;
        m_isDirty = true;
    }

    // Method to check if the transform is dirty
    bool isDirty() const {
        return m_isDirty;
    }

    // Method to clear the dirty flag
    void clearDirtyFlag() {
        m_isDirty = false;
    }

    // Method to calculate and return the model matrix
    glm::mat4 calculateModelMatrix() {
        glm::mat4 model = glm::mat4(1.0f);

        // Apply translation
        model = glm::translate(model, m_position);

        // Apply rotations (YXZ order for Euler angles)
        model = glm::rotate(model, m_eulerAngles.y * DEG_TO_RAD, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, m_eulerAngles.x * DEG_TO_RAD, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, m_eulerAngles.z * DEG_TO_RAD, glm::vec3(0.0f, 0.0f, 1.0f));

        // Apply scaling
        model = glm::scale(model, m_scale);

        return model;
    }

private:
    glm::vec3 m_position;
    glm::vec3 m_scale;
    glm::vec3 m_eulerAngles;
    bool m_isDirty;
};

#endif // TRANSFORM_H
