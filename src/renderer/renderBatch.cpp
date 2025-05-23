#include "renderBatch.h"
#include "renderer.h"
#include <iostream>
#include <map>

RenderBatch::RenderBatch(size_t initialCapacity) {
    m_instances.reserve(initialCapacity);
    m_drawCommands.reserve(initialCapacity);
    m_objectData.reserve(initialCapacity);
    initBuffers(initialCapacity);
}

RenderBatch::~RenderBatch() {
    cleanup();
}

void RenderBatch::addInstance(const Mesh& mesh, const glm::mat4& modelMatrix, const glm::vec2& uvScale) {
    m_instances.push_back({mesh, modelMatrix, uvScale});
}

void RenderBatch::prepare(Renderer& renderer) {
    m_drawCommands.clear();
    m_objectData.clear();

    if (m_instances.empty()) return;

    // Group instances by mesh ID for batching
    std::map<GLuint, std::vector<size_t>> meshGroups;
    for (size_t i = 0; i < m_instances.size(); ++i) {
        GLuint meshId = static_cast<GLuint>(m_instances[i].mesh.id);
        meshGroups[meshId].push_back(i);
    }

    // Build draw commands and object data for each mesh group
    GLuint currentBaseInstance = 0;
    for (const auto& [meshId, instanceIndices] : meshGroups) {
        const Mesh& mesh = m_instances[instanceIndices[0]].mesh; // Use first instance's mesh
        GLuint instanceCount = static_cast<GLuint>(instanceIndices.size());

        // Build draw command with proper instance count
        buildDrawCommand(mesh, renderer, currentBaseInstance, instanceCount);

        // Add all instances for this mesh to object data
        for (size_t instanceIdx : instanceIndices) {
            const auto& instance = m_instances[instanceIdx];

            DrawInstance gpuInstance;
            gpuInstance.modelMatrix = instance.modelMatrix;
            gpuInstance.uvScale = instance.uvScale;
            gpuInstance.padding = glm::vec2(0.0f);

            m_objectData.push_back(gpuInstance);
        }

        currentBaseInstance += instanceCount;
    }

    updateBuffers();
}

void RenderBatch::render(Renderer& renderer) {
    if (m_drawCommands.empty()) {
        return;
    }

    // FIXED: Ensure the SSBO is bound before rendering
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_drawInstanceSSBO);

    renderer.executeIndirectDraw(m_drawCommands, m_indirectBuffer);

    // Unbind SSBO after rendering
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
}

void RenderBatch::clear() {
    m_instances.clear();
}

void RenderBatch::initBuffers(size_t capacity) {
    // FIXED: Ensure proper buffer sizing for mixed command types
    glGenBuffers(1, &m_indirectBuffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectBuffer);
    // Size for the larger of the two command types to handle both
    size_t commandSize = std::max(sizeof(DrawElementsIndirectCommand), sizeof(DrawArraysIndirectCommand));
    glBufferData(GL_DRAW_INDIRECT_BUFFER,
                 capacity * commandSize,
                 nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    m_indirectBufferCapacity = capacity;

    glGenBuffers(1, &m_drawInstanceSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_drawInstanceSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 capacity * sizeof(DrawInstance),
                 nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_drawInstanceSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    m_drawInstanceSSBOCapacity = capacity;
}

void RenderBatch::updateBuffers() {
    // FIXED: Handle buffer resizing for mixed command types
    if (m_drawCommands.size() > m_indirectBufferCapacity) {
        size_t newCapacity = m_drawCommands.size() * 3 / 2;
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectBuffer);
        size_t commandSize = std::max(sizeof(DrawElementsIndirectCommand), sizeof(DrawArraysIndirectCommand));
        glBufferData(GL_DRAW_INDIRECT_BUFFER,
                     newCapacity * commandSize,
                     nullptr, GL_DYNAMIC_DRAW);
        m_indirectBufferCapacity = newCapacity;
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    }

    if (m_objectData.size() > m_drawInstanceSSBOCapacity) {
        size_t newCapacity = m_objectData.size() * 3 / 2;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_drawInstanceSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER,
                     newCapacity * sizeof(DrawInstance),
                     nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_drawInstanceSSBO);
        m_drawInstanceSSBOCapacity = newCapacity;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    // FIXED: Don't upload IndirectDrawCommand directly - let executeIndirectDraw handle it
    // The IndirectDrawCommand contains both types, but OpenGL expects specific command types

    // Upload instance data
    if (!m_objectData.empty()) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_drawInstanceSSBO);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,
                        static_cast<GLsizeiptr>(m_objectData.size() * sizeof(DrawInstance)),
                        m_objectData.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
}

void RenderBatch::buildDrawCommand(const Mesh& mesh, Renderer& renderer, GLuint baseInstance, GLuint instanceCount) {
    IndirectDrawCommand cmd;
    cmd.meshId = static_cast<GLuint>(mesh.id);
    cmd.useIndices = renderer.hasMeshIndices(mesh.id);

    if (cmd.useIndices) {
        GLsizei actualIndexCount = renderer.getMeshIndexCount(mesh.id);
        if (actualIndexCount == 0) {
            std::cerr << "[Error] Mesh " << mesh.id << " has no indices but marked as indexed\n";
            return;
        }

        cmd.elements.count = (mesh.count > 0) ? mesh.count : actualIndexCount;
        cmd.elements.instanceCount = instanceCount; // Set to actual instance count
        cmd.elements.firstIndex = mesh.firstIndex;
        cmd.elements.baseVertex = mesh.baseVertex;
        cmd.elements.baseInstance = baseInstance; // Starting instance index
    } else {
        GLsizei actualVertexCount = renderer.getMeshVertexCount(mesh.id);
        if (actualVertexCount == 0) {
            std::cerr << "[Error] Mesh " << mesh.id << " has no vertices\n";
            return;
        }

        cmd.arrays.count = (mesh.count > 0) ? mesh.count : actualVertexCount;
        cmd.arrays.instanceCount = instanceCount; // Set to actual instance count
        cmd.arrays.first = mesh.firstIndex;
        cmd.arrays.baseInstance = baseInstance; // Starting instance index
    }

    m_drawCommands.push_back(cmd);
}

void RenderBatch::cleanup() {
    if (m_indirectBuffer) {
        glDeleteBuffers(1, &m_indirectBuffer);
        m_indirectBuffer = 0;
    }
    if (m_drawInstanceSSBO) {
        glDeleteBuffers(1, &m_drawInstanceSSBO);
        m_drawInstanceSSBO = 0;
    }
}
