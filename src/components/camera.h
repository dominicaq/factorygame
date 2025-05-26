#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <entt/entt.hpp>
#include "transform.h"

class Camera {
private:
    float m_nearPlane = 0.1f;
    float m_farPlane = 1000.0f;
    float m_fov = 90.0f;
    float m_aspectRatio = 1.0f;

    mutable glm::mat4 m_projection;
    mutable bool m_dirtyProjection = true;

    // View matrix caching
    mutable glm::mat4 m_view;
    mutable glm::vec3 m_lastPosition = glm::vec3(0.0f);
    mutable glm::quat m_lastRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

    // EnTT components
    entt::entity m_entity;
    entt::registry& m_registry;

public:
    // Constructor
    Camera(entt::entity cameraEntity, entt::registry& registry)
        : m_entity(cameraEntity), m_registry(registry) {}

    // Setters that mark projection as dirty
    void setNearPlane(float nearPlane) {
        m_nearPlane = nearPlane;
        m_dirtyProjection = true;
    }

    void setFarPlane(float farPlane) {
        m_farPlane = farPlane;
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

    glm::vec3 getForward() {
        const auto& rotation = m_registry.get<Rotation>(m_entity).quaternion;
        return glm::normalize(rotation * glm::vec3(0.0f, 0.0f, -1.0f));
    }

    glm::vec3 getPosition() const {
        const auto& positionComponent = m_registry.get<Position>(m_entity);
        return positionComponent.position;
    }

    glm::mat4 getViewMatrix() const {
        const auto& position = m_registry.get<Position>(m_entity).position;
        const auto& orientation = m_registry.get<Rotation>(m_entity).quaternion;

        // Check if view matrix needs to be recalculated
        if (position != m_lastPosition || orientation != m_lastRotation) {
            // Convert quaternion to rotation matrix
            glm::mat4 rotationMatrix = glm::mat4_cast(orientation);

            // Get the forward vector from the rotation matrix (typically the -z axis)
            glm::vec3 front = glm::normalize(glm::vec3(rotationMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));

            // Get the up vector from the rotation matrix (typically the y axis)
            glm::vec3 up = glm::normalize(glm::vec3(rotationMatrix * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)));

            m_view = glm::lookAt(position, position + front, up);

            // Update cached values
            m_lastPosition = position;
            m_lastRotation = orientation;
        }

        return m_view;
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
