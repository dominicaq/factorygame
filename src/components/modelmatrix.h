#ifndef MODELMATRIX_H
#define MODELMATRIX_H

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"

#include "ecs/ecs.h"
#include "transform.h"
#include <vector>

struct ModelMatrix {
    glm::mat4 matrix = glm::mat4(1.0f);
    bool dirty = true;
};

// System to update the ModelMatrix component
inline void updateModelMatrices(ECSWorld& ecs, const std::vector<Entity>& entities) {
    for (Entity entity : entities) {
        auto& modelMatrix = ecs.getComponent<ModelMatrix>(entity);

        // Only update if the model matrix is dirty
        if (modelMatrix.dirty) {
            auto& position = ecs.getComponent<PositionComponent>(entity);
            auto& rotation = ecs.getComponent<RotationComponent>(entity);
            auto& scale = ecs.getComponent<ScaleComponent>(entity);

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

#endif // MODELMATRIX_H
