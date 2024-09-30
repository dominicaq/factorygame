#ifndef CAMERA_H
#define CAMERA_H

// Math
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"

struct Camera {
public:
    // Orientation data
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 5.0f);
    glm::vec3 eulerAngles = glm::vec3(0.0f, 0.0f, 0.0f);

private:
    float m_nearPlane = 0.1f;
    float m_farPlane = 100.0f;
    // Field of View in degrees
    float m_fov = 45.0f;
    float m_aspectRatio = 1.0f;

    mutable glm::mat4 m_projection;
    mutable bool m_dirtyProjection = true;

public:
    // Setters that mark projection as dirty
    void setNearPlane(float near) {
        m_nearPlane = near;
        m_dirtyProjection = true;
    }

    void setFarPlane(float far) {
        m_farPlane = far;
        m_dirtyProjection = true;
    }

    void setFov(float fieldOfView) {
        m_fov = fieldOfView;
        m_dirtyProjection = true;
    }

    void setAspectRatio(float width, float height) {
        m_aspectRatio = width / height;
        m_dirtyProjection = true;
    }

    // Getters for accessing private members
    float getNearPlane() const { return m_nearPlane; }
    float getFarPlane() const { return m_farPlane; }
    float getFov() const { return m_fov; }
    float getAspectRatio() const { return m_aspectRatio; }

    glm::mat4 getViewMatrix() const {
        glm::vec3 front;
        front.x = cos(glm::radians(eulerAngles.y)) * cos(glm::radians(eulerAngles.x));
        front.y = sin(glm::radians(eulerAngles.x));
        front.z = sin(glm::radians(eulerAngles.y)) * cos(glm::radians(eulerAngles.x));
        front = glm::normalize(front);

        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::normalize(glm::cross(front, up));
        up = glm::normalize(glm::cross(right, front));

        return glm::lookAt(position, position + front, up);
    }

    glm::mat4 getProjectionMatrix() const {
        if (m_dirtyProjection) {
            m_projection = glm::perspective(
                glm::radians(m_fov),
                m_aspectRatio,
                m_nearPlane,
                m_farPlane);
            m_dirtyProjection = false;
        }
        return m_projection;
    }
};

#endif // CAMERA_H
