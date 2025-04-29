#include "../gltfLoader.h"

void printGLTFInfo(const GLTFDocument& doc) {
    std::cout << "glTF Version: " << doc.asset.version << "\n";
    if (!doc.asset.generator.empty()) {
        std::cout << "Generator: " << doc.asset.generator << "\n";
    }

    std::cout << "Scenes: " << doc.scenes.size() << "\n";
    std::cout << "Nodes: " << doc.nodes.size() << "\n";
    std::cout << "Meshes: " << doc.meshes.size() << "\n";
    std::cout << "Accessors: " << doc.accessors.size() << "\n";
    std::cout << "BufferViews: " << doc.bufferViews.size() << "\n";
    std::cout << "Buffers: " << doc.buffers.size() << "\n";

    // Print information about accessors
    std::cout << "\nAccessor Information:" << "\n";
    for (size_t i = 0; i < doc.accessors.size(); i++) {
        const auto& accessor = doc.accessors[i];
        std::cout << "  Accessor " << i << ":" << "\n";
        std::cout << "    Component Type: " << static_cast<int>(accessor.componentType) << "\n";
        std::cout << "    Count: " << accessor.count << "\n";
        std::cout << "    Element Size: " << accessor.getElementSize() << " bytes" << "\n";

        if (!accessor.min.empty()) {
            std::cout << "    Min: [";
            for (size_t j = 0; j < accessor.min.size(); j++) {
                std::cout << accessor.min[j];
                if (j < accessor.min.size() - 1) std::cout << ", ";
            }
            std::cout << "]" << "\n";
        }

        if (!accessor.max.empty()) {
            std::cout << "    Max: [";
            for (size_t j = 0; j < accessor.max.size(); j++) {
                std::cout << accessor.max[j];
                if (j < accessor.max.size() - 1) std::cout << ", ";
            }
            std::cout << "]" << "\n";
        }
    }
}

int main() {
    std::string assetDir = std::string(GLTF_TEST_ASSETS_DIR);
    GLTFDocument doc = parseGLTF(assetDir + "triangle/triangle.gltf");

    printGLTFInfo(doc);
    return 0;
}
