#include "renderer.h"
#include <iostream>

Renderer::Renderer(unsigned int width, unsigned int height) {
    initOpenGLState();
}

Renderer::~Renderer() {
    // Clean up allocated mesh buffers on the GPU
    for (const auto& entry : m_meshBuffers) {
        deleteMeshBuffer(entry.first);
    }
}

void Renderer::initOpenGLState() {
    glEnable(GL_DEPTH_TEST);

    // Culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Clear color
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

/*
* Meshbuffers
*/
void Renderer::initMeshBuffers(const Mesh* mesh) {
    // Check if the mesh has already been set up
    if (m_meshBuffers.find(mesh) != m_meshBuffers.end()) {
        return;
    }

    // Generate and bind VAO, VBO, and EBO for the mesh
    MeshData data;
    glGenVertexArrays(1, &data.VAO);
    glBindVertexArray(data.VAO);

    // Prepare buffer for vertex data (positions, uvs, normals)
    std::vector<float> bufferData;

    // Prepare normals
    const std::vector<glm::vec3>* normalsToUse = &mesh->normals;
    std::vector<glm::vec3> generatedNormals(mesh->vertices.size(), glm::vec3(0.0f));
    if (mesh->normals.empty() && !mesh->indices.empty()) {
        generateNormals(mesh, generatedNormals, mesh->indices);
        normalsToUse = &generatedNormals;
    }

    // Prepare UVs
    const std::vector<glm::vec2>* uvsToUse = &mesh->uvs;
    std::vector<glm::vec2> generatedUVs;
    if (mesh->uvs.empty()) {
        generateUVs(mesh, generatedUVs);
        uvsToUse = &generatedUVs;
    }

    // Populate buffer data (positions, uvs, normals)
    for (size_t i = 0; i < mesh->vertices.size(); ++i) {
        // Position
        bufferData.push_back(mesh->vertices[i].x);
        bufferData.push_back(mesh->vertices[i].y);
        bufferData.push_back(mesh->vertices[i].z);

        // Use generated or provided UVs
        bufferData.push_back((*uvsToUse)[i].x);
        bufferData.push_back((*uvsToUse)[i].y);

        // Normals (use calculated normals if they weren't provided)
        bufferData.push_back((*normalsToUse)[i].x);
        bufferData.push_back((*normalsToUse)[i].y);
        bufferData.push_back((*normalsToUse)[i].z);
    }

    // Generate and bind VBO
    glGenBuffers(1, &data.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, data.VBO);
    glBufferData(GL_ARRAY_BUFFER, bufferData.size() * sizeof(float), bufferData.data(), GL_STATIC_DRAW);

    // Generate and bind EBO for indices
    if (!mesh->indices.empty()) {
        glGenBuffers(1, &data.EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size() * sizeof(unsigned int), mesh->indices.data(), GL_STATIC_DRAW);
    }

    // Set vertex attribute pointers
    // Positions (location = 0, 3 floats)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    // Normals (location = 1, 3 floats)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));

    // UVs (location = 2, 2 floats)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    // Unbind VAO for now
    glBindVertexArray(0);

    // Store the mesh data in the map
    m_meshBuffers[mesh] = data;
}

void Renderer::deleteMeshBuffer(const Mesh* mesh) {
    // Check if the mesh is in the buffer map
    auto it = m_meshBuffers.find(mesh);
    if (it != m_meshBuffers.end()) {
        const MeshData& target = it->second;

        // Delete the OpenGL buffers
        if (target.VAO) {
            glDeleteVertexArrays(1, &target.VAO);
        }
        if (target.VBO) {
            glDeleteBuffers(1, &target.VBO);
        }
        if (target.EBO) {
            glDeleteBuffers(1, &target.EBO);
        }

        // Remove the mesh from the map
        m_meshBuffers.erase(it);
    } else {
        std::cerr << "ERROR: Mesh buffer not found.\n";
    }
}

void Renderer::draw(const Mesh* mesh, const Shader& shader) {
    // Bind the shader to this call
    shader.use();

    // Load and bind mesh buffer
    const MeshData& targetMesh = m_meshBuffers[mesh];
    glBindVertexArray(targetMesh.VAO);

    // Check if indices are available
    if (!mesh->indices.empty()) {
        // Render the mesh using indices
        GLsizei eboSize = static_cast<GLsizei>(mesh->indices.size());
        glDrawElements(GL_TRIANGLES, eboSize, GL_UNSIGNED_INT, 0);
    } else {
        // Render the mesh using vertex array without indices
        GLsizei vertexCount = static_cast<GLsizei>(mesh->vertices.size());
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }

    // Unbind VAO after rendering
    glBindVertexArray(0);
}

/*
 * Missing mesh data generation
 */
void Renderer::generateNormals(const Mesh* mesh, std::vector<glm::vec3>& normals, const std::vector<unsigned int>& indices) {
    // Generating smooth normals

    normals.clear();
    normals.resize(mesh->vertices.size(), glm::vec3(0.0f));

    // Iterate over triangles
    for (size_t i = 0; i < indices.size(); i += 3) {
        // Get the vertices of the triangle
        unsigned int index0 = indices[i];
        unsigned int index1 = indices[i + 1];
        unsigned int index2 = indices[i + 2];

        glm::vec3 v0 = mesh->vertices[index0];
        glm::vec3 v1 = mesh->vertices[index1];
        glm::vec3 v2 = mesh->vertices[index2];

        // Calculate the two edges of the triangle
        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;

        // Calculate the face normal
        glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

        // Accumulate the face normal for each vertex of the triangle
        normals[index0] += faceNormal;
        normals[index1] += faceNormal;
        normals[index2] += faceNormal;
    }

    // Normalize the accumulated vertex normals
    for (size_t i = 0; i < normals.size(); ++i) {
        normals[i] = glm::normalize(normals[i]);
    }
}

void Renderer::generateUVs(const Mesh* mesh, std::vector<glm::vec2>& uvs) {
    uvs.reserve(mesh->vertices.size());

    // Simple Planar Projection: map vertex positions (x, y) to UVs
    for (const auto& vertex : mesh->vertices) {
        // Generate UVs using planar projection onto the XY plane
        float u = (vertex.x + 1.0f) * 0.5f; // Normalize x to [0, 1]
        float v = (vertex.y + 1.0f) * 0.5f; // Normalize y to [0, 1]

        // Add the generated UV to the uvs vector
        uvs.push_back(glm::vec2(u, v));
    }
}
