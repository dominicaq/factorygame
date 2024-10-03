#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "ecs/entity.h"
#include "ecs/ecsworld.h"
#include "glm.hpp"
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

struct Rotation {
    glm::vec3 eulerAngles = glm::vec3(0.0f);
};

struct Scale {
    glm::vec3 scale = glm::vec3(1.0f);
};

// Define local transformation components (defaults to zero)
struct LocalPosition {
    glm::vec3 position = glm::vec3(0.0f);
};

struct LocalRotation {
    glm::vec3 eulerAngles = glm::vec3(0.0f);
};

struct LocalScale {
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
                         const glm::vec3& rotation = glm::vec3(0.0f),
                         const glm::vec3& scale = glm::vec3(1.0f)) {
    // Add local transformation components (initialized to zero/local defaults)
    world.addComponent(entity, LocalPosition{glm::vec3(0.0f)});
    world.addComponent(entity, LocalRotation{glm::vec3(0.0f)});
    world.addComponent(entity, LocalScale{glm::vec3(1.0f)});

    // Add global transformation components (initialized to the given values)
    world.addComponent(entity, Position{position});
    world.addComponent(entity, Rotation{rotation});
    world.addComponent(entity, Scale{scale});
}

// Define the HierarchySystem for updating world space transformations
namespace Transform {

    // Function to update world space positions, rotations, and scales based on parent-child relationships
    inline void updateChildObjects(ECSWorld& world) {
        // Get all entities with a Parent and local transformation components
        auto entities = world.batchedQuery<Parent, LocalPosition, LocalRotation, LocalScale>();

        for (Entity entity : entities) {
            auto& parentComponent = world.getComponent<Parent>(entity);
            if (parentComponent.parent.isValid()) {
                // Get parent's world space transformations
                const auto& parentPosition = world.getComponent<Position>(parentComponent.parent).position;
                const auto& parentRotation = world.getComponent<Rotation>(parentComponent.parent).eulerAngles;
                const auto& parentScale = world.getComponent<Scale>(parentComponent.parent).scale;

                // Get child's local transformations
                const auto& localPosition = world.getComponent<LocalPosition>(entity);
                const auto& localRotation = world.getComponent<LocalRotation>(entity);
                const auto& localScale = world.getComponent<LocalScale>(entity);

                // Calculate parent's rotation as a quaternion for easier application
                glm::quat parentQuat = glm::quat(glm::radians(parentRotation));

                // Update child's world space position
                auto& position = world.getComponent<Position>(entity);
                auto& rotation = world.getComponent<Rotation>(entity);
                auto& scale = world.getComponent<Scale>(entity);

                // Update child's transform
                position.position = parentPosition + (parentQuat * (localPosition.position * parentScale));
                rotation.eulerAngles = glm::degrees(glm::eulerAngles(parentQuat * glm::quat(glm::radians(localRotation.eulerAngles))));
                scale.scale = localScale.scale * parentScale;
            }
        }
    }

    // System to update the ModelMatrix component
    inline void updateModelMatrices(ECSWorld& ecs, const std::vector<Entity>& entities) {
        for (Entity entity : entities) {
            auto& modelMatrix = ecs.getComponent<ModelMatrix>(entity);

            // Only update if the model matrix is dirty
            if (modelMatrix.dirty) {
                auto& position = ecs.getComponent<Position>(entity);
                auto& rotation = ecs.getComponent<Rotation>(entity);
                auto& scale = ecs.getComponent<Scale>(entity);

                glm::mat4 matrix = glm::mat4(1.0f);
                matrix = glm::translate(matrix, position.position);

                // Apply rotation (Euler angles)
                matrix = glm::rotate(matrix, rotation.eulerAngles.x, glm::vec3(1, 0, 0));
                matrix = glm::rotate(matrix, rotation.eulerAngles.y, glm::vec3(0, 1, 0));
                matrix = glm::rotate(matrix, rotation.eulerAngles.z, glm::vec3(0, 0, 1));

                // Apply scaling
                matrix = glm::scale(matrix, scale.scale);

                // Update the matrix in the ModelMatrix component
                modelMatrix.matrix = matrix;

                // Mark the matrix as clean
                modelMatrix.dirty = false;
            }
        }
    }
}

#endif // TRANSFORM_H
