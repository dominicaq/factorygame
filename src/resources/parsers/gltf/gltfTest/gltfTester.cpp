// #include "../gltfParser.h"
// #include <iostream>

// int main() {
//     std::string assetDir = std::string(GLTF_TEST_ASSETS_DIR);
//     std::vector<Mesh> dummyMeshes;
//     bool success = loadGltf(dummyMeshes, assetDir + "triangle/triangle.gltf");
//     if (!success || dummyMeshes.empty()) {
//         std::cout << "Parse failed\n";
//         return -1;
//     }

//     std::cout << "Parse success\n";
//     for (size_t i = 0; i < dummyMeshes.size(); ++i) {
//         const Mesh& mesh = dummyMeshes[i];
//         std::cout << "Mesh #" << i << ":\n";

//         size_t count = std::min<size_t>(10, mesh.vertices.size());
//         std::cout << "  Vertices (" << mesh.vertices.size() << "):\n";
//         for (size_t j = 0; j < count; ++j) {
//             const glm::vec3& v = mesh.vertices[j];
//             std::cout << "    [" << j << "] (" << v.x << ", " << v.y << ", " << v.z << ")\n";
//         }

//         count = std::min<size_t>(10, mesh.uvs.size());
//         std::cout << "  UVs (" << mesh.uvs.size() << "):\n";
//         for (size_t j = 0; j < count; ++j) {
//             const glm::vec2& uv = mesh.uvs[j];
//             std::cout << "    [" << j << "] (" << uv.x << ", " << uv.y << ")\n";
//         }

//         count = std::min<size_t>(10, mesh.normals.size());
//         std::cout << "  Normals (" << mesh.normals.size() << "):\n";
//         for (size_t j = 0; j < count; ++j) {
//             const glm::vec3& n = mesh.normals[j];
//             std::cout << "    [" << j << "] (" << n.x << ", " << n.y << ", " << n.z << ")\n";
//         }

//         count = std::min<size_t>(10, mesh.indices.size());
//         std::cout << "  Indices (" << mesh.indices.size() << "):\n";
//         for (size_t j = 0; j < count; ++j) {
//             std::cout << "    [" << j << "] " << mesh.indices[j] << "\n";
//         }
//     }

//     return 0;
// }
