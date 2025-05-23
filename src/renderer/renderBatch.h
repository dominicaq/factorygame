#pragma once

#include "../components/mesh.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <cstring>

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
    GLuint meshId;
    bool useIndices;

    IndirectDrawCommand() : meshId(0), useIndices(false) {
        std::memset(&elements, 0, sizeof(elements));
    }
};

class RenderBatch {
public:
    // This matches the shader's InstanceData struct
    struct DrawInstance {
        glm::mat4 modelMatrix;
        glm::vec2 uvScale;
        glm::vec2 padding; // For 16-byte alignment
    };

    // Internal storage for CPU-side data
    struct InstanceInfo {
        Mesh mesh;
        glm::mat4 modelMatrix;
        glm::vec2 uvScale;
    };

    RenderBatch(size_t initialCapacity = 1024);
    ~RenderBatch();

    void addInstance(const Mesh& mesh, const glm::mat4& modelMatrix, const glm::vec2& uvScale);
    void prepare(Renderer& renderer);
    void render(Renderer& renderer);
    void clear();

private:
    std::vector<InstanceInfo> m_instances;      // CPU-side data with Mesh info
    std::vector<IndirectDrawCommand> m_drawCommands;
    std::vector<DrawInstance> m_objectData;     // GPU-side data for SSBO

    GLuint m_indirectBuffer = 0;
    GLuint m_drawInstanceSSBO = 0;
    size_t m_indirectBufferCapacity = 0;
    size_t m_drawInstanceSSBOCapacity = 0;

    void initBuffers(size_t capacity);
    void updateBuffers();
    void buildDrawCommand(const Mesh& mesh, Renderer& renderer, GLuint instanceIndex, GLuint instanceCount);
    void cleanup();
};
