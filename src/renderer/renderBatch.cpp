#include "renderBatch.h"
#include "renderer.h"
#include <iostream>
#include <map>

RenderBatch::RenderBatch(size_t initialCapacity) {
    m_instances.reserve(initialCapacity);
    m_elementsCommands.reserve(initialCapacity);
    m_arraysCommands.reserve(initialCapacity);
    m_objectData.reserve(initialCapacity);
    initBuffers(initialCapacity);
}

RenderBatch::~RenderBatch() {
    cleanup();
}

void RenderBatch::addInstance(const RenderInstance& instance) {
    m_instances.push_back(instance);
}

void RenderBatch::prepare(Renderer& renderer) {
    // Clear both command vectors
    m_elementsCommands.clear();
    m_arraysCommands.clear();
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
        const Mesh& mesh = m_instances[instanceIndices[0]].mesh;
        GLuint instanceCount = static_cast<GLuint>(instanceIndices.size());

        // Build draw command with proper instance count
        buildDrawCommand(mesh, renderer, currentBaseInstance, instanceCount);

        // Convert instances to GPU format
        for (size_t instanceIdx : instanceIndices) {
            m_objectData.push_back(m_instances[instanceIdx].toGPUInstance());
        }

        currentBaseInstance += instanceCount;
    }

    updateBuffers();
}

void RenderBatch::render(Renderer& renderer) {
    if (m_elementsCommands.empty() && m_arraysCommands.empty()) {
        return;
    }

    // Bind SSBO once for the entire batch
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_drawInstanceSSBO);

    // Render elements commands if any
    if (!m_elementsCommands.empty()) {
        renderer.executeIndirectDraw(m_elementsCommands, m_elementsIndirectBuffer);
    }

    // Render arrays commands if any
    if (!m_arraysCommands.empty()) {
        renderer.executeIndirectDraw(m_arraysCommands, m_arraysIndirectBuffer);
    }

    // Unbind SSBO after rendering
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
}

void RenderBatch::clear() {
    m_instances.clear();
}

void RenderBatch::initBuffers(size_t capacity) {
    // Create separate buffer for elements commands
    glGenBuffers(1, &m_elementsIndirectBuffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_elementsIndirectBuffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER,
                 capacity * sizeof(DrawElementsIndirectCommand),
                 nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    m_elementsBufferCapacity = capacity;

    // Create separate buffer for arrays commands
    glGenBuffers(1, &m_arraysIndirectBuffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_arraysIndirectBuffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER,
                 capacity * sizeof(DrawArraysIndirectCommand),
                 nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    m_arraysBufferCapacity = capacity;

    // Create instance data SSBO
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
    // Handle elements buffer resizing
    if (m_elementsCommands.size() > m_elementsBufferCapacity) {
        size_t newCapacity = m_elementsCommands.size() * 3 / 2;
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_elementsIndirectBuffer);
        glBufferData(GL_DRAW_INDIRECT_BUFFER,
                     newCapacity * sizeof(DrawElementsIndirectCommand),
                     nullptr, GL_DYNAMIC_DRAW);
        m_elementsBufferCapacity = newCapacity;
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    }

    // Handle arrays buffer resizing
    if (m_arraysCommands.size() > m_arraysBufferCapacity) {
        size_t newCapacity = m_arraysCommands.size() * 3 / 2;
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_arraysIndirectBuffer);
        glBufferData(GL_DRAW_INDIRECT_BUFFER,
                     newCapacity * sizeof(DrawArraysIndirectCommand),
                     nullptr, GL_DYNAMIC_DRAW);
        m_arraysBufferCapacity = newCapacity;
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    }

    // Handle instance data buffer resizing
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

    // Upload instance data to SSBO
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
            return;
        }

        cmd.elements.count = (mesh.count > 0) ? mesh.count : actualIndexCount;
        cmd.elements.instanceCount = instanceCount;
        cmd.elements.firstIndex = mesh.firstIndex;
        cmd.elements.baseVertex = mesh.baseVertex;
        cmd.elements.baseInstance = baseInstance;

        m_elementsCommands.push_back(cmd);
    } else {
        GLsizei actualVertexCount = renderer.getMeshVertexCount(mesh.id);
        if (actualVertexCount == 0) {
            return;
        }

        cmd.arrays.count = (mesh.count > 0) ? mesh.count : actualVertexCount;
        cmd.arrays.instanceCount = instanceCount;
        cmd.arrays.first = mesh.firstIndex;
        cmd.arrays.baseInstance = baseInstance;

        m_arraysCommands.push_back(cmd);
    }
}

void RenderBatch::cleanup() {
    if (m_elementsIndirectBuffer) {
        glDeleteBuffers(1, &m_elementsIndirectBuffer);
        m_elementsIndirectBuffer = 0;
    }
    if (m_arraysIndirectBuffer) {
        glDeleteBuffers(1, &m_arraysIndirectBuffer);
        m_arraysIndirectBuffer = 0;
    }
    if (m_drawInstanceSSBO) {
        glDeleteBuffers(1, &m_drawInstanceSSBO);
        m_drawInstanceSSBO = 0;
    }
}
