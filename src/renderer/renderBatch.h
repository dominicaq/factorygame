#pragma once

#include "../components/mesh.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <cstring>
#include <iostream>

// Forward declaration
class Renderer;

struct DrawElementsIndirectCommand {
    GLuint count;
    GLuint instanceCount;
    GLuint firstIndex;
    GLuint baseVertex;
    GLuint baseInstance;
};

struct DrawArraysIndirectCommand {
    GLuint count;
    GLuint instanceCount;
    GLuint first;
    GLuint baseInstance;
};

struct IndirectDrawCommand {
    union {
        DrawElementsIndirectCommand elements;
        DrawArraysIndirectCommand arrays;
    };
    size_t meshId;
    bool useIndices;

    IndirectDrawCommand() : meshId(0), useIndices(false) {
        std::memset(&elements, 0, sizeof(elements));
    }
};

// GPU-side instance data
struct DrawInstance {
    glm::mat4 modelMatrix;
    glm::vec2 uvScale;
    uint32_t materialId;
    uint32_t padding;
};

// CPU-side instance data with conversion capability
struct RenderInstance {
    Mesh mesh;
    glm::mat4 modelMatrix;
    glm::vec2 uvScale;

    // Constructor for easy creation
    RenderInstance(const Mesh& m, const glm::mat4& matrix, const glm::vec2& uv = glm::vec2(1.0f))
        : mesh(m), modelMatrix(matrix), uvScale(uv) {}

    // Convert to GPU-ready format
    DrawInstance toGPUInstance() const {
        DrawInstance gpu;
        gpu.modelMatrix = modelMatrix;
        gpu.uvScale = uvScale;
        gpu.materialId = mesh.materialIndex;
        gpu.padding = 0;
        return gpu;
    }
};

class RenderBatch {
public:
    RenderBatch(size_t initialCapacity = 1024);
    ~RenderBatch();

    void addInstance(const RenderInstance& instance);
    void prepare(Renderer& renderer);
    void render(Renderer& renderer);
    void clear();

private:
    std::vector<RenderInstance> m_instances;    // CPU-side data
    std::vector<DrawInstance> m_objectData;     // GPU-side data for SSBO

    // Separate command vectors for different draw types
    std::vector<IndirectDrawCommand> m_elementsCommands;
    std::vector<IndirectDrawCommand> m_arraysCommands;

    // Separate buffers for different command types
    GLuint m_elementsIndirectBuffer = 0;
    GLuint m_arraysIndirectBuffer = 0;
    GLuint m_drawInstanceSSBO = 0;

    // Buffer capacities
    size_t m_elementsBufferCapacity = 0;
    size_t m_arraysBufferCapacity = 0;
    size_t m_drawInstanceSSBOCapacity = 0;

    void initBuffers(size_t capacity);
    void updateBuffers();
    void buildDrawCommand(const Mesh& mesh, Renderer& renderer, GLuint baseInstance, GLuint instanceCount);
    void cleanup();
};
