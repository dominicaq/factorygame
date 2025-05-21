#include "renderer.h"
#include "framebuffer.h"
#include <iostream>
#include <memory>

// Define the number of G-buffer attachments
#define NUM_GATTACHMENTS 5
#define MAX_DRAW_COMMMANDS 1024

Renderer::Renderer(config::GraphicsSettings settings, Camera* camera) {
    config = settings;

    // Viewport
    m_camera = camera;
    m_width = config.display.width;
    m_height = config.display.height;

    initOpenGLState();
    initScreenQuad();

    // Initialize G-Buffer
    m_gBuffer = std::make_unique<Framebuffer>(m_width, m_height, NUM_GATTACHMENTS, true);

    // Init the mesh compute shader
    m_heightCompute.load(ASSET_DIR "shaders/core/heightmap.comp");

    // Make the first mesh instance blank
    m_instanceMeshData.push_back(MeshData{0});

    initIndirectDrawBuffer(MAX_DRAW_COMMMANDS);
}

Renderer::~Renderer() {
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

    // Clean up instance mesh buffers
    for (auto& data : m_instanceMeshData) {
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

    // Clean up quad
    if (m_quadVAO) {
        glDeleteVertexArrays(1, &m_quadVAO);
    }
}

/*
 * OpenGL State Initialization
 */
void Renderer::initOpenGLState() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_DEPTH_CLAMP);
    glDepthFunc(GL_LESS);

    // Face Culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Disable Blending by default
    glDisable(GL_BLEND);
}

/*
* Multi Indirect Draw
*/
void Renderer::addIndirectDrawCommand(const Mesh& mesh) {
    if (mesh.id >= m_meshData.size() || m_meshData[mesh.id].VAO == 0) {
        std::cerr << "[Error] Renderer::addIndirectDrawCommand: Invalid mesh ID: " << mesh.id << "\n";
        return;
    }

    DrawElementsIndirectCommand cmd;

    // Determine how many elements to draw for this mesh
    if (mesh.count > 0) {
        // If an explicit count was provided, use it
        cmd.count = mesh.count;
    } else {
        // Otherwise, determine count based on whether the mesh uses an index buffer
        if (m_meshData[mesh.id].EBO != 0) {
            // Use the number of indices if an Element Buffer Object is present
            cmd.count = m_meshData[mesh.id].indexCount;
        } else {
            // Fallback: use the number of vertices if drawing without indices
            cmd.count = m_meshData[mesh.id].vertexCount;
        }
    }

    cmd.instanceCount = mesh.instanceCount;
    cmd.firstIndex = mesh.firstIndex;
    cmd.baseVertex = mesh.baseVertex;
    cmd.baseInstance = mesh.baseInstance;

    m_indirectCommands.push_back(cmd);
}

void Renderer::initIndirectDrawBuffer(size_t maxDrawCommands) {
    // Clean up existing buffer if any
    if (m_indirectBuffer != 0) {
        glDeleteBuffers(1, &m_indirectBuffer);
    }

    // Create the indirect buffer
    glGenBuffers(1, &m_indirectBuffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectBuffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, maxDrawCommands * sizeof(DrawElementsIndirectCommand),
                 nullptr, GL_DYNAMIC_STORAGE_BIT);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

    // Reserve space for indirect commands
    m_indirectCommands.reserve(maxDrawCommands);
}

void Renderer::updateIndirectDrawBuffer() {
    if (m_indirectCommands.empty()) {
        std::cerr << "[Warning] Renderer::updateIndirectDrawBuffer: No indirect commands to update\n";
        return;
    }

    m_indirectCount = static_cast<GLuint>(m_indirectCommands.size());

    // Update indirect command buffer
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectBuffer);
    glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0,
                   m_indirectCount * sizeof(DrawElementsIndirectCommand),
                   m_indirectCommands.data());

    // Update draw count in parameter buffer (if using dynamic count)
    glBindBuffer(GL_PARAMETER_BUFFER_ARB, m_drawCountBuffer);
    glBufferSubData(GL_PARAMETER_BUFFER_ARB, 0, sizeof(GLuint), &m_indirectCount);

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    glBindBuffer(GL_PARAMETER_BUFFER_ARB, 0);
}

void Renderer::drawMultiIndirect(bool indexed) {
    if (m_indirectCount == 0 || m_indirectBuffer == 0) {
        std::cerr << "[Warning] Renderer::drawMultiIndirect: No indirect commands or buffer not initialized\n";
        return;
    }

    // Bind the indirect buffer
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectBuffer);

    // Multi-draw indirect call
    if (indexed) {
        // For elements (indexed rendering)
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr,
                                  m_indirectCount, sizeof(DrawElementsIndirectCommand));
    } else {
        // For arrays (non-indexed rendering)
        glMultiDrawArraysIndirect(GL_TRIANGLES, nullptr,
                                m_indirectCount, sizeof(DrawElementsIndirectCommand));
    }

    // Unbind the buffer
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
}

void Renderer::clearIndirectCommands() {
    m_indirectCommands.clear();
    m_indirectCount = 0;
}

/*
 * Mesh management
 */
void Renderer::draw(const Mesh* mesh) {
    size_t index = mesh->id;
    if (index >= m_meshData.size() || m_meshData[index].VAO == 0) {
        // TODO: TEMP
        // std::cerr << "[Error] Remderer::draw: Mesh buffer id not found!\n";
        return;
    }

    const MeshData& data = m_meshData[index];

    if (mesh->wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    // Bind VAO
    glBindVertexArray(data.VAO);

    // Draw the mesh
    GLenum drawMode = mesh->wireframe ? GL_LINES : mesh->drawMode;
    if (data.EBO != 0) {
        glDrawElements(drawMode, data.indexCount, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(drawMode, 0, data.vertexCount);
    }

    // Unbind VAO
    glBindVertexArray(0);

    if (mesh->wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void Renderer::drawInstanced(size_t instanceID, bool wireframe) {
    if (instanceID >= m_instanceMeshData.size() || m_instanceMeshData[instanceID].VAO == 0) {
        std::cerr << "[Error] Renderer::drawInstanced: Mesh buffer id not found!\n";
        return;
    }

    const MeshData& data = m_instanceMeshData[instanceID];
    const GLsizei count = data.instanceCount;
    if (count <= 0) {
        std::cerr << "[Warning] Renderer::drawInstanced: Instance count is zero or negative!\n";
        return;
    }

    // Ensure SSBO is bound to the shader
    if (data.instanceSSBO != 0) {
        // Bind the SSBO to the appropriate binding point
        GLuint bindingPoint = 0; // Using instanceID as binding point
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, data.instanceSSBO);

    } else {
        std::cerr << "[Warning] Renderer::drawInstanced: SSBO not initialized for this instance!\n";
    }

    // Add wireframe support
    if (wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    // Bind VAO
    glBindVertexArray(data.VAO);

    // Draw the instanced mesh
    if (data.EBO != 0) {
        glDrawElementsInstanced(GL_TRIANGLES, data.indexCount, GL_UNSIGNED_INT, 0, count);
    } else {
        glDrawArraysInstanced(GL_TRIANGLES, 0, data.vertexCount, count);
    }

    // Unbind VAO
    glBindVertexArray(0);

    // Unbind SSBO after use
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, (GLuint)instanceID, 0);

    // Reset polygon mode if wireframe was enabled
    if (wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void Renderer::initMeshBuffers(Mesh* mesh, bool isStatic, size_t instanceID) {
    if (!mesh) {
        std::cerr << "[Error] Renderer::initMeshBuffers: Mesh pointer is null.\n";
        return;
    }

    if (mesh->uvs.empty() ||
        mesh->normals.empty() ||
        mesh->tangents.empty()) {
        std::cerr << "[Error] Renderer::initMeshBuffers: UVs, normals, and tangent space must be provided.\n";
        return;
    }

    bool isInstance = (instanceID != SIZE_MAX);

    // Check if the instance already exists in m_instanceMeshData
    if (isInstance && instanceID < m_instanceMeshData.size() && m_instanceMeshData[instanceID].VAO != 0) {
        return;
    }

    // Assign an ID if not already assigned
    if (mesh->id == SIZE_MAX) {
        mesh->id = isInstance ? instanceID : m_meshData.size();
    }

    // // Modify the mesh by applying its height map
    // if (mesh->material) {
    //     applyHeightMapCompute(mesh);
    // }

    MeshData data = {};
    glGenVertexArrays(1, &data.VAO);
    glBindVertexArray(data.VAO);

    // Build mesh buffer data (12 floats per vertex)
    std::vector<float> bufferData;
    size_t numVertices = mesh->vertices.size();
    bufferData.reserve(numVertices * 12);

    for (size_t i = 0; i < numVertices; ++i) {
        // Positions (3)
        bufferData.push_back(mesh->vertices[i].x);
        bufferData.push_back(mesh->vertices[i].y);
        bufferData.push_back(mesh->vertices[i].z);
        // UVs (2)
        bufferData.push_back(mesh->uvs[i].x);
        bufferData.push_back(mesh->uvs[i].y);
        // Normals (3)
        bufferData.push_back(mesh->normals[i].x);
        bufferData.push_back(mesh->normals[i].y);
        bufferData.push_back(mesh->normals[i].z);
        // Tangents (4)
        bufferData.push_back(mesh->tangents[i].x);
        bufferData.push_back(mesh->tangents[i].y);
        bufferData.push_back(mesh->tangents[i].z);
        bufferData.push_back(mesh->tangents[i].w);
    }

    glGenBuffers(1, &data.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, data.VBO);
    glBufferData(GL_ARRAY_BUFFER, bufferData.size() * sizeof(float), bufferData.data(), GL_STATIC_DRAW);

    if (!mesh->indices.empty()) {
        glGenBuffers(1, &data.EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size() * sizeof(unsigned int), mesh->indices.data(), GL_STATIC_DRAW);
        data.indexCount = static_cast<GLsizei>(mesh->indices.size());
        data.vertexCount = 0;
    } else {
        data.EBO = 0;
        data.indexCount = 0;
        data.vertexCount = static_cast<GLsizei>(mesh->vertices.size());
    }

    // Vertex attribute pointers (using 12 floats per vertex)
    glEnableVertexAttribArray(0); // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1); // UV
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(2); // Normal
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(5 * sizeof(float)));

    glEnableVertexAttribArray(3); // Tangent (vec4)
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(8 * sizeof(float)));

    glBindVertexArray(0);

    // Store mesh data in the appropriate vector
    if (isInstance) {
        // Ensure m_instanceMeshData is large enough
        if (instanceID >= m_instanceMeshData.size()) {
            m_instanceMeshData.resize(instanceID + 1);
        }
        m_instanceMeshData[instanceID] = data;

    } else {
        // Find the first available slot in m_meshData
        bool inserted = false;
        for (size_t i = 0; i < m_meshData.size(); ++i) {
            if (m_meshData[i].VAO == 0) {
                m_meshData[i] = data;
                mesh->id = i;
                inserted = true;
                break;
            }
        }
        if (!inserted) {
            m_meshData.push_back(data);
            mesh->id = m_meshData.size() - 1;
        }
    }

    // Clear CPU-side mesh data if possible
    if (isStatic) {
        mesh->clearData();
    }
}

void Renderer::deleteMeshBuffer(const Mesh* mesh) {
    size_t index = mesh->id;
    if (index >= m_meshData.size() || m_meshData[index].VAO == 0) {
        std::cerr << "[Error] Renderer::deleteMeshBuffer: Mesh buffer id not found!\n";
        return;
    }

    MeshData& data = m_meshData[index];

    // Delete mesh resources
    if (data.VAO) {
        glDeleteVertexArrays(1, &data.VAO);
        data.VAO = 0;
    }
    if (data.VBO) {
        glDeleteBuffers(1, &data.VBO);
        data.VBO = 0;
    }
    if (data.EBO) {
        glDeleteBuffers(1, &data.EBO);
        data.EBO = 0;
    }

    // Reset mesh at index
    m_meshData[index] = MeshData{};
}

/*
 * Screen Quad Initialization and Drawing
 */
void Renderer::initScreenQuad() {
    // Vertices for a screen-aligned quad (NDC space)
    float quadVertices[] = {
        // positions        // uvs
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f
    };

    unsigned int quadIndices[] = {
        0, 1, 2,
        2, 3, 0
    };

    // Generate and bind VAO
    glGenVertexArrays(1, &m_quadVAO);
    glBindVertexArray(m_quadVAO);

    // Generate and bind VBO
    unsigned int VBO, EBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    // Generate and bind EBO
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), &quadIndices, GL_STATIC_DRAW);

    // Position attribute (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    // UV attribute (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    // Unbind VAO
    glBindVertexArray(0);
}

void Renderer::updateInstanceBuffer(size_t instanceID, const std::vector<glm::mat4>& modelMatrices) {
    if (instanceID >= m_instanceMeshData.size() || m_instanceMeshData[instanceID].VAO == 0) {
        std::cerr << "[Error] Renderer::updateInstanceBuffer: Instance mesh not found!\n";
        return;
    }

    MeshData& data = m_instanceMeshData[instanceID];

    if (data.instanceSSBO == 0) {
        std::cerr << "[Error] Renderer::updateInstanceBuffer: Instance SSBO doesn't exist! Call setupInstanceAttributes first.\n";
        return;
    }

    GLuint newSize = (GLuint)(modelMatrices.size() * sizeof(glm::mat4));
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, data.instanceSSBO);

    // Allocate or reallocate if necessary
    if (newSize > data.instanceBufferSize) {
        glBufferData(GL_SHADER_STORAGE_BUFFER, newSize, nullptr, GL_DYNAMIC_DRAW);
        data.instanceBufferSize = newSize;
    }

    // Update the data using glBufferSubData instead of mapping
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, newSize, modelMatrices.data());

    // For very large data sets, consider double-buffering or persistent mapping
    // which would be another optimization beyond this initial SSBO implementation

    data.instanceCount = static_cast<GLsizei>(modelMatrices.size());

    // Update the binding point if needed
    GLuint bindingPoint = 0;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, data.instanceSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Renderer::setupInstanceAttributes(size_t instanceID, const std::vector<glm::mat4>& modelMatrices) {
    if (instanceID >= m_instanceMeshData.size() || m_instanceMeshData[instanceID].VAO == 0) {
        std::cerr << "[Error] Renderer::setupInstanceAttributes: Instance mesh not found!\n";
        return;
    }

    MeshData& data = m_instanceMeshData[instanceID];

    // Create SSBO
    glGenBuffers(1, &data.instanceSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, data.instanceSSBO);

    // Allocate and initialize
    GLuint dataSize = (GLuint)(modelMatrices.size() * sizeof(glm::mat4));
    glBufferData(GL_SHADER_STORAGE_BUFFER, dataSize, modelMatrices.data(), GL_DYNAMIC_DRAW);
    data.instanceBufferSize = dataSize;
    data.instanceCount = static_cast<GLsizei>(modelMatrices.size());

    // Bind to shader binding point (you'll need to match this in your shader)
    GLuint bindingPoint = 0; // Use instanceID as the binding point, or your own system
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, data.instanceSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Renderer::deleteInstanceBuffer(size_t instanceID) {
    if (instanceID >= m_instanceMeshData.size() || m_instanceMeshData[instanceID].VAO == 0) {
        std::cerr << "[Error] Renderer::deleteInstanceBuffer: Instance buffer id not found!\n";
        return;
    }

    MeshData& data = m_instanceMeshData[instanceID];

    // Delete mesh resources
    if (data.VAO) {
        glDeleteVertexArrays(1, &data.VAO);
        data.VAO = 0;
    }
    if (data.VBO) {
        glDeleteBuffers(1, &data.VBO);
        data.VBO = 0;
    }
    if (data.EBO) {
        glDeleteBuffers(1, &data.EBO);
        data.EBO = 0;
    }

    // Reset mesh at index
    m_instanceMeshData[instanceID] = MeshData{};
}

void Renderer::drawScreenQuad() {
    glBindVertexArray(m_quadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Renderer::resize(int width, int height) {
    m_width = width;
    m_height = height;

    // Resize the G-buffer (and in the future, all framebuffers)
    m_gBuffer->resize(width, height);
    if (m_camera) {
        m_camera->setAspectRatio(static_cast<float>(width), static_cast<float>(height));
    }
}

/*
* Misc
*/
int Renderer::getNumAttachments()  { return NUM_GATTACHMENTS; }

bool Renderer::applySettings(const config::GraphicsSettings& settings) {
    bool requiresRestart = false;

    config = settings;

    return requiresRestart;
}

/*
* Mesh Pre Processing
*/
void Renderer::applyHeightMapCompute(Mesh* mesh) {
    if (!mesh->material || !mesh->material->heightMap) {
        return; // No mesh or height map, nothing to do
    }

    // Make sure we have normals
    if (mesh->normals.size() != mesh->vertices.size()) {
        std::cerr << "[Error] Cannot apply height map: mesh is missing normals\n";
        return;
    }

    // Define sensible defaults if not set
    if (mesh->material->heightScale <= 0.0f) {
        mesh->material->heightScale = 1.0f; // Default to 1.0 if not set or negative
    }

    if (mesh->material->uvScale.x <= 0.0f || mesh->material->uvScale.y <= 0.0f) {
        mesh->material->uvScale = glm::vec2(1.0f, 1.0f); // Default to (1,1) if not set properly
    }

    // Activate compute shader
    m_heightCompute.use();

    // Create SSBOs
    GLuint ssboVertexData, ssboUVData;
    glGenBuffers(1, &ssboVertexData);
    glGenBuffers(1, &ssboUVData);

    // Set up vertex data SSBO
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboVertexData);
    glBufferData(GL_SHADER_STORAGE_BUFFER, mesh->vertices.size() * sizeof(glm::vec3), mesh->vertices.data(), GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboVertexData);

    // Set up UV data SSBO
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboUVData);
    glBufferData(GL_SHADER_STORAGE_BUFFER, mesh->uvs.size() * sizeof(glm::vec2), mesh->uvs.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboUVData);

    // Unbind the current SSBO to avoid confusion
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Bind height map texture
    mesh->material->heightMap->bind(0);
    m_heightCompute.setInt("heightMap", 0);
    mesh->material->normalMap->bind(1);
    m_heightCompute.setInt("normalMap", 1);

    // Send uniforms to shader
    m_heightCompute.setFloat("heightScale", mesh->material->heightScale);
    m_heightCompute.setVec2("uvScale", mesh->material->uvScale);

    // Dispatch compute shader
    int workGroupSize = 32;
    GLuint numVertices = static_cast<GLuint>(mesh->vertices.size());
    GLuint numWorkGroups = (numVertices + workGroupSize - 1) / workGroupSize;
    glDispatchCompute(numWorkGroups, 1, 1);

    // Ensure execution is complete before reading back data
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Explicitly rebind the vertex buffer before reading from it
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboVertexData);

    // Now read back the data
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, mesh->vertices.size() * sizeof(glm::vec3), mesh->vertices.data());

    // Clean up
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glDeleteBuffers(1, &ssboVertexData);
    glDeleteBuffers(1, &ssboUVData);
    glUseProgram(0);

    // If you want to recalculate normals after displacement
    // recalculateNormals(mesh);
}
