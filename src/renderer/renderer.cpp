#include "renderer.h"
#include "framebuffer.h"
#include <iostream>
#include <memory>

// Define the number of G-buffer attachments
#define NUM_ATTACHMENTS 4

Renderer::Renderer(int width, int height, int atlasSize, int atlasTileSize, Camera* camera) {
    // Viewport
    m_camera = camera;
    m_width = width;
    m_height = height;

    // Shadow Atlas
    m_atlasSize = atlasSize;
    m_atlasTileSize = atlasTileSize;

    initOpenGLState();
    initScreenQuad();

    // Initialize G-Buffer
    m_gBuffer = std::make_unique<Framebuffer>(width, height, NUM_ATTACHMENTS, true);

    // Initialize Shadow Atlas
    m_shadowAtlas = std::make_unique<Framebuffer>(atlasSize, atlasSize, 0, true);
    m_shadowAtlas->bind();

    // Set up the depth texture parameters for the shadow map
    glBindTexture(GL_TEXTURE_2D, m_shadowAtlas->getDepthAttachment());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    m_shadowAtlas->unbind();

    // Make the first mesh instance blank
    m_instanceMeshData.push_back(MeshData{0});
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
    glDepthFunc(GL_LESS);

    // Face Culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Disable Blending by default
    glDisable(GL_BLEND);
}

/*
 * Mesh management
 */
void Renderer::draw(const Mesh* mesh) {
    size_t index = mesh->id;
    if (index >= m_meshData.size() || m_meshData[index].VAO == 0) {
        std::cerr << "[Error] Remderer::draw: Mesh buffer id not found!\n";
        return;
    }

    const MeshData& data = m_meshData[index];

    // Bind VAO
    glBindVertexArray(data.VAO);

    // Draw the mesh
    if (data.EBO != 0) {
        glDrawElements(GL_TRIANGLES, data.indexCount, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, data.vertexCount);
    }

    // Unbind VAO
    glBindVertexArray(0);
}

void Renderer::drawInstanced(size_t index, size_t instanceCount) {
    if (index >= m_instanceMeshData.size() || m_instanceMeshData[index].VAO == 0) {
        std::cerr << "[Error] Renderer::drawInstanced: Mesh buffer id not found!\n";
        return;
    }

    const MeshData& data = m_instanceMeshData[index];

    // Bind VAO
    glBindVertexArray(data.VAO);

    // Draw the instanced mesh
    if (data.EBO != 0) {
        glDrawElementsInstanced(GL_TRIANGLES, data.indexCount, GL_UNSIGNED_INT, 0, instanceCount);
    } else {
        glDrawArraysInstanced(GL_TRIANGLES, 0, data.vertexCount, instanceCount);
    }

    // Unbind VAO
    glBindVertexArray(0);
}

void Renderer::initMeshBuffers(Mesh* mesh, bool isStatic, size_t instanceID) {
    if (mesh->uvs.empty() ||
        mesh->normals.empty() ||
        mesh->tangents.empty() ||
        mesh->bitangents.empty()) {
        std::cerr << "[Error] Renderer::initMeshBuffers: UVs, normals, tangents, and bitangents must be provided for all vertices.\n";
        return;
    }

    bool isInstance = (instanceID != SIZE_MAX);

    // Check if the instance already exists in m_instanceMeshData
    if (isInstance && m_instanceMeshData[instanceID].VAO != 0) {
        return;
    }

    // Assign an ID if not already assigned
    if (mesh->id == SIZE_MAX) {
        mesh->id = isInstance ? instanceID : m_meshData.size();
    }

    MeshData data = {};
    glGenVertexArrays(1, &data.VAO);
    glBindVertexArray(data.VAO);

    // Build mesh buffer data (14 floats per vertex)
    std::vector<float> bufferData;
    size_t numVertices = mesh->vertices.size();
    bufferData.reserve(numVertices * 14);

    for (size_t i = 0; i < numVertices; ++i) {
        // Positions
        bufferData.push_back(mesh->vertices[i].x);
        bufferData.push_back(mesh->vertices[i].y);
        bufferData.push_back(mesh->vertices[i].z);
        // UVs
        bufferData.push_back(mesh->uvs[i].x);
        bufferData.push_back(mesh->uvs[i].y);
        // Normals
        bufferData.push_back(mesh->normals[i].x);
        bufferData.push_back(mesh->normals[i].y);
        bufferData.push_back(mesh->normals[i].z);
        // Tangents
        bufferData.push_back(mesh->tangents[i].x);
        bufferData.push_back(mesh->tangents[i].y);
        bufferData.push_back(mesh->tangents[i].z);
        // Bitangents
        bufferData.push_back(mesh->bitangents[i].x);
        bufferData.push_back(mesh->bitangents[i].y);
        bufferData.push_back(mesh->bitangents[i].z);
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
        // No indices
        data.EBO = 0;
        data.indexCount = 0;
        data.vertexCount = static_cast<GLsizei>(mesh->vertices.size());
    }

    // Vertex attribute pointers
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(5 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));

    glBindVertexArray(0); // Unbind VAO

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

void Renderer::drawScreenQuad() {
    glBindVertexArray(m_quadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Renderer::resizeGBuffer(int width, int height) {
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
int Renderer::getNumAttachments()  { return NUM_ATTACHMENTS; }
