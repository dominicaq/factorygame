#ifndef SCENE_UTILS_H
#define SCENE_UTILS_H

#include <string>
#include <glm/glm.hpp>
#include "sceneData.h"
#include "engine.h"

enum class GizmoType {
    AXIS,
    CUBE,
    PLANE
};

class SceneUtils {
public:
    // =========================================================================
    // Component Helpers
    // =========================================================================
    /*
     * Adds light components to the specified entity in the registry.
     * @param registry - The registry to add the components to.
     * @param entity - The entity to which the components will be added.
     * @param lightData - The light data to associate with the entity.
     * @param isSpot - Attaches LightSpaceMatrixArray instead of LightSpaceMatrix for directional shaodow map cascades and point lights
     */
    static void addLightComponents(entt::registry& registry, entt::entity entity, Light lightData);

    /*
     * Adds a GameObject component to the specified entity in the registry.
     * Associates the provided scene data with the entity, including transform and meta data.
     * @param registry - The registry to add the component to.
     * @param entity - The entity to which the component will be added.
     * @param data - The scene data to associate with the entity.
     * @return The newly added GameObject component.
     */
    static GameObject* addGameObjectComponent(entt::registry& registry, entt::entity entity, const SceneData& data);

    // =========================================================================
    // GameObject Helpers
    // =========================================================================
    /*
     * Creates an empty GameObject in the registry and associates it with the provided scene data.
     * @param registry - The registry to create the GameObject in.
     * @param data - The scene data to associate with the GameObject.
     */
    static void createEmptyGameObject(entt::registry& registry, const SceneData& data);

    /*
     * Creates a gizmo GameObject in the registry based on the provided scene data and primitive type.
     * @param registry - The registry to create the GameObject in.
     * @param data - The scene data to associate with the GameObject.
     * @param mat - The material to apply to the gizmo mesh.
     * @param type - The type of primitive to create (Axis, Cube, or Plane).
     * @return The entity handle of the created gizmo.
     */
    static entt::entity createGizmo(entt::registry& registry, const SceneData& data, Material* mat, GizmoType type);

    /*
     * Creates a model GameObject in the registry and associates it with the provided scene data and mesh.
     * @param registry - The registry to create the GameObject in.
     * @param data - The scene data to associate with the GameObject.
     * @param mesh - The mesh to associate with the GameObject.
     */
    static void createModel(entt::registry& registry, const SceneData& data, Mesh* mesh);
};

#endif // SCENE_UTILS_H
