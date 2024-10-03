#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "ecs/entity.h"
#include "ecs/ecsworld.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/quaternion.hpp"
#include <vector>

struct ModelMatrix {
    glm::mat4 matrix = glm::mat4(1.0f);
    bool dirty = true;
};

// Define transformation components for world space
struct Position {
    glm::vec3 position = glm::vec3(0.0f);
};

// Internal rotation representation using quaternions
struct Rotation {
    glm::quat quaternion = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
};

// User-facing EulerAngles component (Pitch, Yaw, Roll in degrees)
struct EulerAngles {
    glm::vec3 euler = glm::vec3(0.0f);  // Euler angles in degrees (pitch, yaw, roll)
};

struct Scale {
    glm::vec3 scale = glm::vec3(1.0f);
};

// Define parent-child relationship components
struct Parent {
    Entity parent;
};

struct Children {
    std::vector<Entity> children;
};

// Helper function to add transformation components to an entity
inline void addTransform(ECSWorld& world, Entity entity,
                         const glm::vec3& position = glm::vec3(0.0f),
                         const glm::vec3& rotationEuler = glm::vec3(0.0f),
                         const glm::vec3& scale = glm::vec3(1.0f)) {
    world.addComponent(entity, Position{position});
    world.addComponent(entity, EulerAngles{rotationEuler});
    world.addComponent(entity, Rotation{glm::quat(glm::radians(rotationEuler))});
    world.addComponent(entity, Scale{scale});
}

// Function to update the quaternion from Euler angles (whenever Euler angles are changed)
inline void updateQuaternionFromEuler(ECSWorld& world, Entity entity) {
    auto& eulerAngles = world.getComponent<EulerAngles>(entity).euler;
    auto& rotation = world.getComponent<Rotation>(entity).quaternion;

    // Update quaternion from Euler angles
    rotation = glm::quat(glm::radians(eulerAngles));
}

// Define the HierarchySystem for updating world space transformations
namespace Transform {

    // Function to update world space positions, rotations, and scales based on parent-child relationships
    inline void updateChildObjects(ECSWorld& world) {
        // Get all entities with a Parent and local transformation components
        auto entities = world.batchedQuery<Parent, ModelMatrix>();

        for (Entity entity : entities) {
            auto& parentComponent = world.getComponent<Parent>(entity);
            if (parentComponent.parent.isValid()) {
                // Check if either the parent or the child has a dirty matrix
                auto& modelMatrix = world.getComponent<ModelMatrix>(entity);
                const auto& parentModelMatrix = world.getComponent<ModelMatrix>(parentComponent.parent);

                if (!modelMatrix.dirty && !parentModelMatrix.dirty) {
                    continue; // Skip if no transformation has changed
                }

                // Get parent's world space transformations
                const auto& parentPosition = world.getComponent<Position>(parentComponent.parent).position;
                const auto& parentRotation = world.getComponent<Rotation>(parentComponent.parent).quaternion;
                const auto& parentScale = world.getComponent<Scale>(parentComponent.parent).scale;

                // Get child's local transformations
                auto& localPosition = world.getComponent<Position>(entity).position;
                auto& localRotation = world.getComponent<Rotation>(entity).quaternion;
                auto& localScale = world.getComponent<Scale>(entity).scale;

                // Update child's world space position
                localPosition = parentPosition + (parentRotation * (localPosition * parentScale));

                // Update child's world space rotation (combine quaternions)
                localRotation = parentRotation * localRotation;

                // Update child's world space scale
                localScale *= parentScale;

                // Mark the child entity's model matrix as dirty
                modelMatrix.dirty = true;
            }
        }
    }

    // System to update the ModelMatrix component if marked as dirty
    inline void updateModelMatrices(ECSWorld& world, const std::vector<Entity>& entities) {
        for (Entity entity : entities) {
            auto& modelMatrix = world.getComponent<ModelMatrix>(entity);

            // Only update if the model matrix is dirty
            if (modelMatrix.dirty) {
                auto& position = world.getComponent<Position>(entity).position;
                auto& rotation = world.getComponent<Rotation>(entity).quaternion; // Use quaternion for rotation
                auto& scale = world.getComponent<Scale>(entity).scale;

                // Initialize matrix as identity
                glm::mat4 matrix = glm::mat4(1.0f);

                // Apply position translation
                matrix = glm::translate(matrix, position);

                // Apply rotation (using quaternion)
                matrix *= glm::mat4_cast(rotation);  // Convert quaternion to matrix

                // Apply scale
                matrix = glm::scale(matrix, scale);

                // Update the matrix in the ModelMatrix component
                modelMatrix.matrix = matrix;

                // Mark the matrix as clean
                modelMatrix.dirty = false;
            }
        }
    }

    // Optional function to calculate world direction vectors (forward, up, right)
    inline glm::vec3 getForward(ECSWorld& world, Entity entity) {
        auto& rotation = world.getComponent<Rotation>(entity).quaternion;
        return rotation * glm::vec3(0, 0, 1);  // Rotate the forward direction by the quaternion
    }

    inline glm::vec3 getUp(ECSWorld& world, Entity entity) {
        auto& rotation = world.getComponent<Rotation>(entity).quaternion;
        return rotation * glm::vec3(0, 1, 0);  // Rotate the up direction by the quaternion
    }

    inline glm::vec3 getRight(ECSWorld& world, Entity entity) {
        auto& rotation = world.getComponent<Rotation>(entity).quaternion;
        return rotation * glm::vec3(1, 0, 0);  // Rotate the right direction by the quaternion
    }
}

#endif // TRANSFORM_H
