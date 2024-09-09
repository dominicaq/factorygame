#ifndef MESH_LOADER_H
#define MESH_LOADER_H

#include <vector>
#include <string>
#include "mesh.h"

class MeshLoader {
public:
    // Enumeration for supported file types (currently only OBJ)
    enum class FileType {
        OBJ,
        GLTF // Placeholder for future support
    };

    // Loads a mesh from a file with the specified type (currently OBJ only)
    bool loadMesh(const std::string& filepath, FileType type, Mesh& mesh, int* status);

private:
    // Helper functions to load specific file formats
    bool loadOBJ(const std::string& filepath, Mesh& mesh, int* status);
    // Placeholder for future GLTF loading functionality
    bool loadGLTF(const std::string& filepath, Mesh& mesh, int* status);

    // Helper function to parse a line in the OBJ file
    void parseOBJLine(const std::string& line, Mesh& mesh);
};

#endif // MESH_LOADER_H
