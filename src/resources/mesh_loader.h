#ifndef MESH_LOADER_H
#define MESH_LOADER_H

#include <vector>
#include <string>
#include "mesh.h"

namespace MeshLoader {
    // Enumeration for supported file types
    enum class FileType {
        OBJ,    // Wavefront OBJ format
        GLTF    // Placeholder for GLTF format (not yet implemented)
    };

    // Main function to load a mesh from a file of the specified type
    Mesh* loadMesh(const std::string& filepath, FileType type);

    // Helper function to load OBJ files specifically
    Mesh* loadOBJ(const std::string& filepath);

    // Helper function to parse a line from the OBJ file
    void parseOBJLine(const std::string& line, Mesh& mesh);
}

#endif // MESH_LOADER_H
