#ifndef FRAMEGRAPH_H
#define FRAMEGRAPH_H
#include <memory>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include "renderpass.h"
#include "entt/entt.hpp"

// Forward declaration
class Mesh;

class FrameGraph {
public:
    // Add a render pass to the frame graph
    void addRenderPass(std::unique_ptr<RenderPass> pass) {
        pass->setFrameGraph(this);
        m_renderPasses.push_back(std::move(pass));
    }

    // Set up all render passes in the frame graph
    void setupPasses() {
        for (auto& pass : m_renderPasses) {
            pass->setup();
        }
    }

    // Execute all render passes, passing in the renderer and registry
    void executePasses(Renderer& renderer, entt::registry& registry) {
        // Clear and rebuild the instance map once per frame
        if (!m_updateOnce) {
            m_updateOnce = true;
            updateInstanceMap(registry);
        }

        // Execute each pass with access to the shared instance map
        for (auto& pass : m_renderPasses) {
            pass->execute(renderer, registry);
        }
    }

    // Getter for the shared instance map
    const std::unordered_map<size_t, std::vector<glm::mat4>>& getInstanceMap() const {
        return m_instanceMap;
    }

    // Getter for the mesh instances
    const std::vector<Mesh*>& getMeshInstances() const {
        return m_meshInstances;
    }

    // Set the mesh instances
    void setMeshInstances(const std::vector<Mesh*>& meshInstances) {
        m_meshInstances = meshInstances;
    }

private:
    // Instance data
    std::unordered_map<size_t, std::vector<glm::mat4>> m_instanceMap;
    std::vector<Mesh*> m_meshInstances;
    bool m_updateOnce = false;

    // Pass data
    std::vector<std::unique_ptr<RenderPass>> m_renderPasses;

    // Update the instance map with current frame data
    void updateInstanceMap(entt::registry& registry) {
        m_instanceMap.clear();

        // Collect all instances by mesh ID
        registry.view<MeshInstance, ModelMatrix>().each([&](MeshInstance& instance, const ModelMatrix& modelMatrix) {
            size_t id = instance.id;
            m_instanceMap[id].push_back(modelMatrix.matrix);
        });
    }
};

#endif // FRAMEGRAPH_H
