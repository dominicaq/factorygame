#ifndef CAMERA_H
#define CAMERA_H

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "ecs/ecs.h"
#include "transform.h"

class Camera {
private:
    Entity m_cameraEntity;  // The ECS entity for the camera
    ECSWorld* m_world;      // Reference to the ECS world

    float m_nearPlane = 0.1f;
    float m_farPlane = 100.0f;
    float m_fov = 45.0f;
    float m_aspectRatio = 1.0f;

    mutable glm::mat4 m_projection;
    mutable bool m_dirtyProjection = true;

public:
    // Constructor
    Camera(Entity cameraEntity, ECSWorld* world)
        : m_cameraEntity(cameraEntity), m_world(world) {}

    // Setters and getters for position
    glm::vec3 getPosition() const {
        const auto& positionComponent = m_world->getComponent<Position>(m_cameraEntity);
        return positionComponent.position;
    }

    void setPosition(const glm::vec3& newPosition) {
        auto& positionComponent = m_world->getComponent<Position>(m_cameraEntity);
        positionComponent.position = newPosition;
    }

    // Setters and getters for rotation
    glm::vec3 getRotation() const {
        const auto& rotationComponent = m_world->getComponent<Rotation>(m_cameraEntity);
        return rotationComponent.eulerAngles;
    }

    void setRotation(const glm::vec3& newRotation) {
        auto& rotationComponent = m_world->getComponent<Rotation>(m_cameraEntity);
        rotationComponent.eulerAngles = newRotation;
    }

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
        glm::vec3 rotation = getRotation();
        glm::vec3 position = getPosition();

        glm::vec3 front;
        front.x = cos(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
        front.y = sin(glm::radians(rotation.x));
        front.z = sin(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
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
