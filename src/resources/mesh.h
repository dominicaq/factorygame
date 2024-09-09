#ifndef MESH_H
#define MESH_H

#include <vector>
#include <glm.hpp>

struct Mesh {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;
};

#endif // MESH_H
