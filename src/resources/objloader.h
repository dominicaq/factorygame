// #ifndef OBJLOADER_H
// #define OBJLOADER_H

// #include <string>
// #include <vector>
// #include <iostream>
// #include <fstream>
// #include <sstream>
// #include <unordered_map>
// #include <tuple>
// #include <glm.hpp>
// #include "mesh.h" // Include your Mesh struct

// // Function to load an OBJ file and return a Mesh object
// inline Mesh* loadOBJ(const std::string& filepath) {
//     // Temporary storage for parsing
//     std::vector<glm::vec3> temp_positions;
//     std::vector<glm::vec2> temp_uvs;
//     std::vector<glm::vec3> temp_normals;

//     struct Face {
//         unsigned int positionIndices[3];
//         unsigned int uvIndices[3];
//         unsigned int normalIndices[3];
//     };

//     std::vector<Face> faces;

//     // Read the OBJ file content
//     std::ifstream file(filepath);
//     if (!file.is_open()) {
//         std::cerr << "ERROR::OBJLOADER::Failed to read OBJ file: " << filepath << "\n";
//         return nullptr;
//     }

//     std::string line;
//     while (std::getline(file, line)) {
//         if (line.empty()) {
//             continue;
//         }

//         // Parse each line
//         std::istringstream sstream(line);
//         std::string type;
//         sstream >> type;

//         if (type == "v") {
//             // Vertex position
//             glm::vec3 vertex;
//             sstream >> vertex.x >> vertex.y >> vertex.z;
//             temp_positions.push_back(vertex);
//         } else if (type == "vt") {
//             // Texture coordinates (UVs)
//             glm::vec2 uv;
//             sstream >> uv.x >> uv.y;
//             temp_uvs.push_back(uv);
//         } else if (type == "vn") {
//             // Vertex normal
//             glm::vec3 normal;
//             sstream >> normal.x >> normal.y >> normal.z;
//             temp_normals.push_back(normal);
//         } else if (type == "f") {
//             // Face
//             Face face;
//             std::string vertexData;

//             // Process the 3 vertices that make up a triangle face
//             for (int i = 0; i < 3; i++) {
//                 if (!(sstream >> vertexData)) {
//                     std::cerr << "ERROR::OBJLOADER::Invalid face data in file: " << filepath << "\n";
//                     // delete mesh;
//                     return nullptr;
//                 }

//                 std::stringstream faceStream(vertexData);
//                 std::string indexToken;
//                 int count = 0;

//                 // Initialize indices to max value to indicate missing data
//                 face.positionIndices[i] = std::numeric_limits<unsigned int>::max();
//                 face.uvIndices[i] = std::numeric_limits<unsigned int>::max();
//                 face.normalIndices[i] = std::numeric_limits<unsigned int>::max();

//                 // Parse the vertex/uv/normal indices
//                 while (std::getline(faceStream, indexToken, '/')) {
//                     if (!indexToken.empty()) {
//                         unsigned int index = static_cast<unsigned int>(std::stoi(indexToken)) - 1; // OBJ indices start at 1
//                         if (count == 0)
//                             face.positionIndices[i] = index;
//                         else if (count == 1)
//                             face.uvIndices[i] = index;
//                         else if (count == 2)
//                             face.normalIndices[i] = index;
//                     }
//                     count++;
//                 }
//             }
//             faces.push_back(face);
//         }
//     }

//     file.close();

//     // Reconstruct mesh with unique vertices
//     Mesh* mesh = new Mesh();
//     std::unordered_map<std::tuple<unsigned int, unsigned int, unsigned int>, unsigned int> vertexToIndex;

//     for (const auto& face : faces) {
//         for (int i = 0; i < 3; i++) {
//             unsigned int positionIndex = face.positionIndices[i];
//             unsigned int uvIndex = face.uvIndices[i];
//             unsigned int normalIndex = face.normalIndices[i];

//             // Create a key for the vertex combination
//             auto key = std::make_tuple(positionIndex, uvIndex, normalIndex);

//             auto it = vertexToIndex.find(key);
//             if (it != vertexToIndex.end()) {
//                 // Vertex already exists
//                 mesh->indices.push_back(it->second);
//             } else {
//                 // New unique vertex
//                 glm::vec3 position = (positionIndex != std::numeric_limits<unsigned int>::max() && positionIndex < temp_positions.size())
//                     ? temp_positions[positionIndex]
//                     : glm::vec3(0.0f);

//                 glm::vec2 uv = (uvIndex != std::numeric_limits<unsigned int>::max() && uvIndex < temp_uvs.size())
//                     ? temp_uvs[uvIndex]
//                     : glm::vec2(0.0f);

//                 glm::vec3 normal = (normalIndex != std::numeric_limits<unsigned int>::max() && normalIndex < temp_normals.size())
//                     ? temp_normals[normalIndex]
//                     : glm::vec3(0.0f);

//                 mesh->vertices.push_back(position);
//                 mesh->uvs.push_back(uv);
//                 mesh->normals.push_back(normal);

//                 unsigned int newIndex = static_cast<unsigned int>(mesh->vertices.size()) - 1;
//                 mesh->indices.push_back(newIndex);
//                 vertexToIndex[key] = newIndex;
//             }
//         }
//     }

//     return mesh;
// }

// /*
// * Generator functions
// */
// inline void generateNormals(Mesh* mesh) {
//     for (size_t i = 0; i < mesh->indices.size(); i += 3) {
//         unsigned int index0 = mesh->indices[i];
//         unsigned int index1 = mesh->indices[i + 1];
//         unsigned int index2 = mesh->indices[i + 2];

//         glm::vec3& v0 = mesh->vertices[index0];
//         glm::vec3& v1 = mesh->vertices[index1];
//         glm::vec3& v2 = mesh->vertices[index2];

//         glm::vec3 edge1 = v1 - v0;
//         glm::vec3 edge2 = v2 - v0;
//         glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

//         mesh->normals[index0] += faceNormal;
//         mesh->normals[index1] += faceNormal;
//         mesh->normals[index2] += faceNormal;
//     }

//     // Normalize the normals
//     for (size_t i = 0; i < mesh->normals.size(); ++i) {
//         mesh->normals[i] = glm::normalize(mesh->normals[i]);
//     }
// }

// inline void generateUVs(Mesh* mesh) {
//     for (size_t i = 0; i < mesh->vertices.size(); ++i) {
//         float u = (mesh->vertices[i].x + 1.0f) * 0.5f;
//         float v = (mesh->vertices[i].y + 1.0f) * 0.5f;
//         mesh->uvs[i] = glm::vec2(u, v);
//     }
// }

// #endif // OBJLOADER_H
