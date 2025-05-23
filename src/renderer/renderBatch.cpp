#include "renderBatch.h"
#include "renderer.h"  // Now we can include the full renderer header
#include <iostream>

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

    for (const auto& instance : m_instances) {
        buildDrawCommand(instance.mesh, renderer); // Pass renderer

        // Create GPU-side instance data (matches shader struct)
        DrawInstance gpuInstance;
        gpuInstance.modelMatrix = instance.modelMatrix;
        gpuInstance.uvScale = instance.uvScale;
        gpuInstance.padding = glm::vec2(0.0f); // Zero padding for alignment

        m_objectData.push_back(gpuInstance);
    }

    updateBuffers();
}

void RenderBatch::render(Renderer& renderer) {
    if (m_drawCommands.empty()) {
        return;
    }
    renderer.executeIndirectDraw(m_drawCommands, m_indirectBuffer);
}

void RenderBatch::clear() {
    m_instances.clear();
}

void RenderBatch::initBuffers(size_t capacity) {
    glGenBuffers(1, &m_indirectBuffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectBuffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER,
                 capacity * sizeof(IndirectDrawCommand),
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
    if (m_drawCommands.size() > m_indirectBufferCapacity) {
        size_t newCapacity = m_drawCommands.size() * 3 / 2;
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectBuffer);
        glBufferData(GL_DRAW_INDIRECT_BUFFER,
                     newCapacity * sizeof(IndirectDrawCommand),
                     nullptr, GL_DYNAMIC_DRAW);
        m_indirectBufferCapacity = newCapacity;
    }

    if (m_objectData.size() > m_drawInstanceSSBOCapacity) {
        size_t newCapacity = m_objectData.size() * 3 / 2;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_drawInstanceSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER,
                     newCapacity * sizeof(DrawInstance),
                     nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_drawInstanceSSBO);
        m_drawInstanceSSBOCapacity = newCapacity;
    }

    if (!m_drawCommands.empty()) {
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectBuffer);
        glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0,
                        static_cast<GLsizeiptr>(m_drawCommands.size() * sizeof(IndirectDrawCommand)),
                        m_drawCommands.data());
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    }

    if (!m_objectData.empty()) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_drawInstanceSSBO);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,
                        static_cast<GLsizeiptr>(m_objectData.size() * sizeof(DrawInstance)),
                        m_objectData.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
}

void RenderBatch::buildDrawCommand(const Mesh& mesh, Renderer& renderer) {
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
        cmd.elements.instanceCount = 1;
        cmd.elements.firstIndex = mesh.firstIndex;
        cmd.elements.baseVertex = mesh.baseVertex;

        // FIXED: baseInstance should match the instance data index
        cmd.elements.baseInstance = static_cast<GLuint>(m_objectData.size()); // Use objectData size, not drawCommands
    } else {
        GLsizei actualVertexCount = renderer.getMeshVertexCount(mesh.id);
        if (actualVertexCount == 0) {
            std::cerr << "[Error] Mesh " << mesh.id << " has no vertices\n";
            return;
        }

        cmd.arrays.count = (mesh.count > 0) ? mesh.count : actualVertexCount;
        cmd.arrays.instanceCount = 1;
        cmd.arrays.first = mesh.firstIndex;

        // FIXED: baseInstance should match the instance data index
        cmd.arrays.baseInstance = static_cast<GLuint>(m_objectData.size()); // Use objectData size, not drawCommands
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
