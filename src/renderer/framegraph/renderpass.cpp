#include "renderpass.h"
#include "framegraph.h"

const std::unordered_map<size_t, std::vector<glm::mat4>>& RenderPass::getInstanceMap() const {
    if (!m_frameGraph) {
        static std::unordered_map<size_t, std::vector<glm::mat4>> emptyMap;
        return emptyMap;
    }
    return m_frameGraph->getInstanceMap();
}

const std::vector<Mesh*>& RenderPass::getMeshInstances() const {
    if (!m_frameGraph) {
        static std::vector<Mesh*> emptyVec;
        return emptyVec;
    }
    return m_frameGraph->getMeshInstances();
}
