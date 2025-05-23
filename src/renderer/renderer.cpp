#include "renderer.h"
#include <iostream>

#define VERTEX_SIZE 8
#define NUM_GATTACHMENTS 5
#define MAX_DRAW_COMMANDS 1024

Renderer::Renderer(config::GraphicsSettings settings, Camera* camera)
    : config(settings), m_camera(camera) {

    m_width = config.display.width;
    m_height = config.display.height;

    initOpenGLState();
    initScreenQuad();

    // Initialize G-Buffer
    m_gBuffer = std::make_unique<Framebuffer>(m_width, m_height, NUM_GATTACHMENTS, true);

    initIndirectDrawBuffer(MAX_DRAW_COMMANDS);
}

Renderer::~Renderer() {
    cleanup();
}

// ============================================================================
// COMMAND BUFFER MANAGEMENT (from your original code, cleaned up)
// ============================================================================

void Renderer::clearDrawCommands() {
    m_commandBuffer.clear();
    m_indirectCount = 0;
}

void Renderer::addDrawCommand(const Mesh& mesh) {
    // Enhanced validation
    if (mesh.id >= m_meshData.size() || m_meshData[mesh.id].VAO == 0) {
        std::cerr << "[Error] Invalid mesh ID: " << mesh.id << "\n";
        return;
    }

    const MeshData& meshData = m_meshData[mesh.id];

    // Create unified command
    IndirectDrawCommand cmd;
    cmd.meshId = mesh.id;
    cmd.useIndices = (meshData.EBO != 0);

    if (cmd.useIndices) {
        uint32_t requestedCount = mesh.count > 0 ? mesh.count : meshData.indexCount;
        uint32_t requestedFirst = mesh.firstIndex;

        if (requestedFirst >= meshData.indexCount) {
            return;
        }

        if (requestedFirst + requestedCount > meshData.indexCount) {
            requestedCount = meshData.indexCount - requestedFirst;
        }

        cmd.elements.count = requestedCount;
        cmd.elements.instanceCount = mesh.instanceCount > 0 ? mesh.instanceCount : 1;
        cmd.elements.firstIndex = requestedFirst;
        cmd.elements.baseVertex = mesh.baseVertex;
        cmd.elements.baseInstance = mesh.baseInstance;
    } else {
        uint32_t requestedCount = mesh.count > 0 ? mesh.count : meshData.vertexCount;
        uint32_t requestedFirst = mesh.firstIndex;

        if (requestedFirst >= meshData.vertexCount) {
            return;
        }

        if (requestedFirst + requestedCount > meshData.vertexCount) {
            requestedCount = meshData.vertexCount - requestedFirst;
        }

        cmd.arrays.count = requestedCount;
        cmd.arrays.instanceCount = mesh.instanceCount > 0 ? mesh.instanceCount : 1;
        cmd.arrays.first = requestedFirst;
        cmd.arrays.baseInstance = mesh.baseInstance;
    }

    m_commandBuffer.push_back(cmd);
}


// ============================================================================
// MESH MANAGEMENT (from your original code)
// ============================================================================

Mesh& Renderer::initMeshBuffers(std::unique_ptr<RawMeshData>& rawData, bool isStatic) {
    static Mesh invalidMesh; // Return this for errors

    if (rawData->uvs.empty() || rawData->packedTNBFrame.empty()) {
        std::cerr << "[Error] Renderer::initMeshBuffers: UVs and tangent space must be provided.\n";
        return invalidMesh;
    }

    MeshData data = {};
    glGenVertexArrays(1, &data.VAO);
    glBindVertexArray(data.VAO);

    // Build mesh buffer data (VERTEX_SIZE = 8 floats per vertex)
    std::vector<float> bufferData;
    size_t numVertices = rawData->vertices.size();
    bufferData.reserve(numVertices * VERTEX_SIZE);

    for (size_t i = 0; i < numVertices; ++i) {
        // Positions (3 floats)
        bufferData.push_back(rawData->vertices[i].x);
        bufferData.push_back(rawData->vertices[i].y);
        bufferData.push_back(rawData->vertices[i].z);

        // Packed UVs (1 float)
        uint16_t u = static_cast<uint16_t>(glm::clamp(rawData->uvs[i].x, 0.0f, 1.0f) * 65535.0f);
        uint16_t v = static_cast<uint16_t>(glm::clamp(rawData->uvs[i].y, 0.0f, 1.0f) * 65535.0f);
        uint32_t packedUV = (static_cast<uint32_t>(v) << 16) | u;
        bufferData.push_back(*reinterpret_cast<float*>(&packedUV));

        // Packed Normal & Tangent frame (4 floats)
        bufferData.push_back(rawData->packedTNBFrame[i].x);
        bufferData.push_back(rawData->packedTNBFrame[i].y);
        bufferData.push_back(rawData->packedTNBFrame[i].z);
        bufferData.push_back(rawData->packedTNBFrame[i].w);
    }

    static Mesh newMesh;

    // Create and upload VBO
    glGenBuffers(1, &data.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, data.VBO);
    glBufferData(GL_ARRAY_BUFFER, bufferData.size() * sizeof(float),
                 bufferData.data(), isStatic ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);

    // Handle indices
    if (!rawData->indices.empty()) {
        glGenBuffers(1, &data.EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, rawData->indices.size() * sizeof(unsigned int),
                     rawData->indices.data(), GL_STATIC_DRAW);
        data.indexCount = static_cast<GLsizei>(rawData->indices.size());
        data.vertexCount = 0;
        newMesh.count = static_cast<uint32_t>(data.indexCount);
    } else {
        data.EBO = 0;
        data.indexCount = 0;
        data.vertexCount = static_cast<GLsizei>(rawData->vertices.size());
        newMesh.count = static_cast<uint32_t>(data.vertexCount);
    }

    // Attribute 0: Position (xyz) + Packed UVs (w) = 4 floats
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VERTEX_SIZE * sizeof(float), (void*)0);

    // Attribute 1: Packed Normal & Tangent frame = 4 floats
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VERTEX_SIZE * sizeof(float), (void*)(4 * sizeof(float)));

    glBindVertexArray(0);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "[Error] OpenGL error during mesh buffer creation: " << error << "\n";
        glDeleteVertexArrays(1, &data.VAO);
        if (data.VBO) glDeleteBuffers(1, &data.VBO);
        if (data.EBO) glDeleteBuffers(1, &data.EBO);
        return invalidMesh;
    }

    // Find available slot or add new one
    size_t assignedId = SIZE_MAX;
    for (size_t i = 0; i < m_meshData.size(); ++i) {
        if (m_meshData[i].VAO == 0) {
            m_meshData[i] = data;
            assignedId = i;
            break;
        }
    }

    if (assignedId == SIZE_MAX) {
        m_meshData.push_back(data);
        assignedId = m_meshData.size() - 1;
    }

    newMesh.id = assignedId;

    // Clear CPU data if static
    if (isStatic) {
        rawData->clearData();
    }
    return newMesh;
}

GLuint Renderer::getMeshVAO(size_t meshId) const {
    if (meshId >= m_meshData.size()) return 0;
    return m_meshData[meshId].VAO;
}

bool Renderer::hasMeshIndices(size_t meshId) const {
    if (meshId >= m_meshData.size()) return false;
    return m_meshData[meshId].EBO != 0;
}

GLsizei Renderer::getMeshIndexCount(size_t meshId) const {
    if (meshId >= m_meshData.size()) return 0;
    return m_meshData[meshId].indexCount;
}

GLsizei Renderer::getMeshVertexCount(size_t meshId) const {
    if (meshId >= m_meshData.size()) return 0;
    return m_meshData[meshId].vertexCount;
}

// ============================================================================
// UTILITY IMPLEMENTATIONS
// ============================================================================

void Renderer::initOpenGLState() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_DEPTH_CLAMP);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glDisable(GL_BLEND);
}

void Renderer::initScreenQuad() {
    float quadVertices[] = {
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f
    };

    unsigned int quadIndices[] = { 0, 1, 2, 2, 3, 0 };

    glGenVertexArrays(1, &m_quadVAO);
    glBindVertexArray(m_quadVAO);

    unsigned int VBO, EBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), &quadIndices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
}

void Renderer::drawScreenQuad() {
    glBindVertexArray(m_quadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Renderer::initIndirectDrawBuffer(size_t maxDrawCommands) {
    // Clean up existing buffers
    if (m_indirectBuffer != 0) {
        glDeleteBuffers(1, &m_indirectBuffer);
    }
    if (m_drawCountBuffer != 0) {
        glDeleteBuffers(1, &m_drawCountBuffer);
    }

    // Create indirect command buffer (for both elements and arrays)
    glGenBuffers(1, &m_indirectBuffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectBuffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER,
                 maxDrawCommands * sizeof(DrawElementsIndirectCommand),
                 nullptr, GL_DYNAMIC_DRAW);

    // Create buffer for draw count (bind as regular buffer for now)
    glGenBuffers(1, &m_drawCountBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_drawCountBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_commandBuffer.reserve(maxDrawCommands);
}

void Renderer::cleanup() {
    // Clean up mesh buffers
    for (auto& data : m_meshData) {
        if (data.VAO) {
            glDeleteVertexArrays(1, &data.VAO);
        }
        if (data.VBO) {
            glDeleteBuffers(1, &data.VBO);
        }
        if (data.EBO) {
            glDeleteBuffers(1, &data.EBO);
        }
    }

    // Clean up indirect draw resources
    if (m_indirectBuffer != 0) {
        glDeleteBuffers(1, &m_indirectBuffer);
        m_indirectBuffer = 0;
    }

    if (m_drawCountBuffer != 0) {
        glDeleteBuffers(1, &m_drawCountBuffer);
        m_drawCountBuffer = 0;
    }

    // Clean up screen quad
    if (m_quadVAO) {
        glDeleteVertexArrays(1, &m_quadVAO);
        m_quadVAO = 0;
    }
}

void Renderer::resize(int width, int height) {
    m_width = width;
    m_height = height;

    if (m_gBuffer) {
        m_gBuffer->resize(width, height);
    }

    if (m_camera) {
        m_camera->setAspectRatio(static_cast<float>(width), static_cast<float>(height));
    }
}

int Renderer::getNumAttachments() {
    return NUM_GATTACHMENTS;
}

bool Renderer::applySettings(const config::GraphicsSettings& settings) {
    bool requiresRestart = false;
    config = settings;
    return requiresRestart;
}

void Renderer::executeIndirectDraw(const std::vector<IndirectDrawCommand>& commands, GLuint indirectBuffer) {
    if (commands.empty() || indirectBuffer == 0) {
        return;
    }

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer);

    // Since we now have separate buffers, all commands are the same type
    bool isIndexed = commands[0].useIndices;

    if (isIndexed) {
        // Upload elements commands
        std::vector<DrawElementsIndirectCommand> elementCommands;
        elementCommands.reserve(commands.size());
        for (const auto& cmd : commands) {
            elementCommands.push_back(cmd.elements);
        }

        glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0,
                       elementCommands.size() * sizeof(DrawElementsIndirectCommand),
                       elementCommands.data());

        // Draw in batches by VAO
        GLuint currentVAO = 0;
        size_t batchStart = 0;

        for (size_t i = 0; i <= commands.size(); ++i) {
            GLuint nextVAO = (i < commands.size()) ? getMeshVAO(commands[i].meshId) : 0;

            // Draw current batch if VAO changes or we're at the end
            if ((nextVAO != currentVAO || i == commands.size()) && currentVAO != 0) {
                glBindVertexArray(currentVAO);

                size_t batchSize = i - batchStart;
                size_t offsetBytes = batchStart * sizeof(DrawElementsIndirectCommand);

                glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT,
                                          reinterpret_cast<const void*>(offsetBytes),
                                          static_cast<GLsizei>(batchSize),
                                          sizeof(DrawElementsIndirectCommand));
                batchStart = i;
            }
            currentVAO = nextVAO;
        }
    } else {
        // Upload arrays commands
        std::vector<DrawArraysIndirectCommand> arrayCommands;
        arrayCommands.reserve(commands.size());
        for (const auto& cmd : commands) {
            arrayCommands.push_back(cmd.arrays);
        }

        glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0,
                       arrayCommands.size() * sizeof(DrawArraysIndirectCommand),
                       arrayCommands.data());

        // Draw in batches by VAO
        GLuint currentVAO = 0;
        size_t batchStart = 0;

        for (size_t i = 0; i <= commands.size(); ++i) {
            GLuint nextVAO = (i < commands.size()) ? getMeshVAO(commands[i].meshId) : 0;

            // Draw current batch if VAO changes or we're at the end
            if ((nextVAO != currentVAO || i == commands.size()) && currentVAO != 0) {
                glBindVertexArray(currentVAO);

                size_t batchSize = i - batchStart;
                size_t offsetBytes = batchStart * sizeof(DrawArraysIndirectCommand);

                glMultiDrawArraysIndirect(GL_TRIANGLES,
                                        reinterpret_cast<const void*>(offsetBytes),
                                        static_cast<GLsizei>(batchSize),
                                        sizeof(DrawArraysIndirectCommand));
                batchStart = i;
            }
            currentVAO = nextVAO;
        }
    }

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    glBindVertexArray(0);
}
