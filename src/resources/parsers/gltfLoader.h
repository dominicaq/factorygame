#pragma once

#include <json/json.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstring>

// glTF component type enum (matches WebGL/OpenGL constants)
enum class GLTFComponentType : int {
    BYTE = 5120,
    UNSIGNED_BYTE = 5121,
    SHORT = 5122,
    UNSIGNED_SHORT = 5123,
    UNSIGNED_INT = 5125,
    FLOAT = 5126
};

// Accessor type enum and related utility functions
enum class GLTFAccessorType {
    SCALAR,
    VEC2,
    VEC3,
    VEC4,
    MAT2,
    MAT3,
    MAT4,
    UNKNOWN
};

// Convert string accessor type to enum
static inline GLTFAccessorType stringToAccessorType(const std::string& type) {
    if (type == "SCALAR") return GLTFAccessorType::SCALAR;
    if (type == "VEC2") return GLTFAccessorType::VEC2;
    if (type == "VEC3") return GLTFAccessorType::VEC3;
    if (type == "VEC4") return GLTFAccessorType::VEC4;
    if (type == "MAT2") return GLTFAccessorType::MAT2;
    if (type == "MAT3") return GLTFAccessorType::MAT3;
    if (type == "MAT4") return GLTFAccessorType::MAT4;
    return GLTFAccessorType::UNKNOWN;
}

// Get number of components for each accessor type
static inline int getComponentCount(GLTFAccessorType type) {
    switch (type) {
        case GLTFAccessorType::SCALAR: return 1;
        case GLTFAccessorType::VEC2: return 2;
        case GLTFAccessorType::VEC3: return 3;
        case GLTFAccessorType::VEC4: return 4;
        case GLTFAccessorType::MAT2: return 4; // 2x2 matrix
        case GLTFAccessorType::MAT3: return 9; // 3x3 matrix
        case GLTFAccessorType::MAT4: return 16; // 4x4 matrix
        default: return 0;
    }
}

// Get size in bytes for component types
static inline int getComponentTypeSize(GLTFComponentType componentType) {
    switch (componentType) {
        case GLTFComponentType::BYTE:
        case GLTFComponentType::UNSIGNED_BYTE:
            return 1;
        case GLTFComponentType::SHORT:
        case GLTFComponentType::UNSIGNED_SHORT:
            return 2;
        case GLTFComponentType::UNSIGNED_INT:
        case GLTFComponentType::FLOAT:
            return 4;
        default:
            return 0;
    }
}

/*
* Base64 encode / decode
*/
static inline std::string base64Encode(const std::string& input) {
    // Base64 character set
    const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string encoded_string;
    size_t input_length = input.length();
    size_t i = 0;

    while (i < input_length) {
        uint32_t chunk = 0;
        size_t j = 0;

        // Create a 24-bit chunk from the input
        for (; j < 3 && i < input_length; ++j, ++i) {
            chunk |= static_cast<uint32_t>(input[i]) << (16 - j * 8);
        }

        // Encode the chunk into 4 base64 characters
        for (j = 0; j < 4; ++j) {
            if (j * 6 > 24) {
                break;
            }
            encoded_string += base64_chars[(chunk >> (18 - j * 6)) & 0x3F];
        }
    }

    // Add padding if necessary
    size_t padding_count = (3 - (input_length % 3)) % 3;
    for (size_t k = 0; k < padding_count; ++k) {
        encoded_string += '=';
    }

    return encoded_string;
}

static inline std::string base64Decode(const std::string& encoded_string) {
    // Base64 character set (used for reverse lookup)
    const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string decoded_string;
    size_t input_length = encoded_string.length();
    size_t i = 0;

    while (i < input_length) {
        uint32_t chunk = 0;
        size_t j = 0;

        // Create a 24-bit chunk from the encoded input
        for (; j < 4 && i < input_length; ++j, ++i) {
            if (encoded_string[i] == '=') {
                break;
            }

            size_t char_index = base64_chars.find(encoded_string[i]);
            if (char_index == std::string::npos) {
                 throw std::runtime_error("Invalid base64 character");
            }
            chunk |= static_cast<uint32_t>(char_index) << (18 - j * 6);
        }

        // Decode the chunk into bytes
        for (j = 0; j < 3; ++j) {
            if (j * 8 >= 24) {
                break;
            }
            decoded_string += static_cast<char>((chunk >> (16 - j * 8)) & 0xFF);
        }
    }
    return decoded_string;
}

/*
* glTF 2.0 structure
*/
struct GLTFAsset {
    std::string version;
    std::string generator;
    std::string copyright;
};

struct GLTFScene {
    std::string name;
    std::vector<int> nodes;
};

struct GLTFNode {
    std::string name;
    std::vector<int> children;

    // Transform type enum to indicate which transform is being used
    enum TransformType {
        MATRIX,
        COMPONENTS,
        NONE
    };

    TransformType transformType = NONE;

    // Matrix (when using matrix transform)
    std::vector<float> matrix;

    // Transform Components
    std::vector<float> translation;
    std::vector<float> scale;
    std::vector<float> rotation;  // quaternion (x,y,z,w)

    int mesh = -1;  // Index to mesh or -1
};

struct GLTFAccessor {
    // Required properties
    int count = 0;                           // Number of elements
    GLTFComponentType componentType = GLTFComponentType::FLOAT; // Data type of components
    GLTFAccessorType type = GLTFAccessorType::SCALAR;  // Type of elements in the accessor

    // Optional properties
    int bufferView = -1;                     // Index of the bufferView (-1 = undefined)
    size_t byteOffset = 0;                   // Offset into the buffer in bytes
    bool normalized = false;                 // Whether integer values should be normalized
    std::vector<float> min;                  // Minimum value of each component
    std::vector<float> max;                  // Maximum value of each component
    std::string name;                        // Optional name

    // Sparse storage of attributes that deviate from their initialization value
    struct Sparse {
        int count = 0;                       // Number of sparse elements

        struct Indices {
            int bufferView;                  // Index of the bufferView with indices
            size_t byteOffset = 0;           // Offset into the bufferView in bytes
            GLTFComponentType componentType; // Data type of indices
            std::string extras_json_string;
            std::string extensions_json_string;
        } indices;

        struct Values {
            int bufferView;                  // Index of the bufferView with values
            size_t byteOffset = 0;           // Offset into the bufferView in bytes
            std::string extras_json_string;
            std::string extensions_json_string;
        } values;

        std::string extras_json_string;
        std::string extensions_json_string;
    };

    std::optional<Sparse> sparse;            // Optional sparse storage
    std::string extras_json_string;
    std::string extensions_json_string;

    // Helper methods

    // Get byte size of a single element
    size_t getElementSize() const {
        return getComponentCount(type) * getComponentTypeSize(componentType);
    }

    // Get total byte size of the accessor data
    size_t getTotalSize() const {
        return count * getElementSize();
    }

    // Check if this accessor has sparse data
    bool isSparse() const {
        return sparse.has_value() && sparse->count > 0;
    }
};

struct GLTFBufferView {
    int buffer = 0;
    int byteOffset = 0;
    int byteLength = 0;
    int byteStride = 0;  // Optional, stride between vertex attributes
    int target = 0;      // GL enum: 34962/34963 (ARRAY_BUFFER/ELEMENT_ARRAY_BUFFER)
};

struct GLTFBuffer {
    std::string uri;
    int byteLength = 0;
    std::vector<unsigned char> data;  // Binary data if loaded
};

struct GLTFPrimitive {
    std::unordered_map<std::string, int> attributes;  // Attribute name to accessor index
    int indices = -1;      // Accessor index for indices or -1
    int material = -1;     // Material index or -1
    int mode = 4;          // Default is TRIANGLES (4)
};

struct GLTFMesh {
    std::string name;
    std::vector<GLTFPrimitive> primitives;
    std::vector<float> weights;  // For morph targets
};

struct GLTFDocument {
    GLTFAsset asset;
    std::vector<GLTFScene> scenes;
    std::vector<GLTFNode> nodes;
    std::vector<GLTFMesh> meshes;
    std::vector<GLTFAccessor> accessors;
    std::vector<GLTFBufferView> bufferViews;
    std::vector<GLTFBuffer> buffers;
    int defaultScene = 0;
    // We'll add more components as we go
};

using json = nlohmann::json;

// Utility template functions for more efficient parsing
template<typename T>
void parse_if_exists(const json& j, const char* key, T& target) {
    auto it = j.find(key);
    if (it != j.end()) {
        target = it->get<T>();
    }
}

template<typename T>
void parse_array_if_exists(const json& j, const char* key, std::vector<T>& target) {
    auto it = j.find(key);
    if (it != j.end() && it->is_array()) {
        target.reserve(it->size()); // Pre-allocate memory
        for (const auto& elem : *it) {
            target.push_back(elem.get<T>());
        }
    }
}

// Function to load buffer data from URI (either embedded or external file)
void loadBufferData(GLTFDocument& doc, const std::string& baseDir = "") {
    for (auto& buffer : doc.buffers) {
        if (buffer.uri.empty()) {
            // This might be a GLB file with embedded buffer - we would need to handle that separately
            continue;
        }

        // Check if it's a data URI (base64)
        if (buffer.uri.substr(0, 5) == "data:") {
            size_t commaPos = buffer.uri.find(',');
            if (commaPos != std::string::npos) {
                std::string encodedData = buffer.uri.substr(commaPos + 1);
                std::string decodedData = base64Decode(encodedData);

                if (!decodedData.empty()) {
                    buffer.data.resize(decodedData.size());
                    buffer.data = std::vector<uint8_t>(decodedData.begin(), decodedData.end());
                } else {
                    std::cerr << "Error decoding base64 URI.\n";
                    buffer.data.clear();
                }
                continue;
            }
        } else {
            // It's a relative path to an external file
            std::string fullPath = baseDir + buffer.uri;
            std::ifstream file(fullPath, std::ios::binary);

            if (!file.is_open()) {
                std::cerr << "Failed to open buffer file: " << fullPath << "\n";
                buffer.data.clear();
                continue;
            }

            buffer.data.resize(buffer.byteLength);
            if (!file.read(reinterpret_cast<char*>(buffer.data.data()), buffer.byteLength)) {
                std::cerr << "Failed to read all bytes from buffer file: " << fullPath << "\n";
                buffer.data.clear();
            }
            file.close();
        }
    }
}

GLTFDocument parseGLTF(const std::string& filename) {
    GLTFDocument doc;

    // Open and read the file in one go
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open glTF file: " << filename << "\n";
        return doc;
    }

    // Read the entire file into memory at once
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string buffer(size, ' ');
    file.read(&buffer[0], size);

    // Parse the JSON
    json j = json::parse(buffer);

    // Parse asset info
    if (j.contains("asset")) {
        const auto& asset = j["asset"];
        parse_if_exists(asset, "version", doc.asset.version);
        parse_if_exists(asset, "generator", doc.asset.generator);
        parse_if_exists(asset, "copyright", doc.asset.copyright);
    }

    // Parse scenes
    if (j.contains("scenes")) {
        const auto& scenes = j["scenes"];
        doc.scenes.reserve(scenes.size());

        for (const auto& scene : scenes) {
            GLTFScene s;
            parse_if_exists(scene, "name", s.name);
            parse_array_if_exists(scene, "nodes", s.nodes);
            doc.scenes.push_back(std::move(s));
        }
    }

    // Parse nodes
    if (j.contains("nodes")) {
        const auto& nodes = j["nodes"];
        doc.nodes.reserve(nodes.size());

        for (const auto& node : nodes) {
            GLTFNode n;
            parse_if_exists(node, "name", n.name);
            parse_if_exists(node, "mesh", n.mesh);
            parse_array_if_exists(node, "children", n.children);
            parse_array_if_exists(node, "matrix", n.matrix);
            if (n.matrix.size() > 1) {
                n.transformType = GLTFNode::TransformType::MATRIX;
            } else {
                parse_array_if_exists(node, "translation", n.translation);
                parse_array_if_exists(node, "rotation", n.rotation);
                parse_array_if_exists(node, "scale", n.scale);
                n.transformType = GLTFNode::TransformType::COMPONENTS;
            }
            doc.nodes.push_back(std::move(n));
        }
    }

    // Parse accessors with enhanced type handling
    if (j.contains("accessors")) {
        const auto& accessors = j["accessors"];
        doc.accessors.reserve(accessors.size());

        for (const auto& accessor : accessors) {
            GLTFAccessor a;
            parse_if_exists(accessor, "bufferView", a.bufferView);
            parse_if_exists(accessor, "byteOffset", a.byteOffset);

            // Parse componentType as integer and convert to enum
            if (accessor.contains("componentType")) {
                a.componentType = static_cast<GLTFComponentType>(accessor["componentType"].get<int>());
            }

            parse_if_exists(accessor, "normalized", a.normalized);
            parse_if_exists(accessor, "count", a.count);

            // Parse type string and convert to enum
            if (accessor.contains("type")) {
                std::string typeString = accessor["type"].get<std::string>();
                a.type = stringToAccessorType(typeString);
            }

            parse_array_if_exists(accessor, "min", a.min);
            parse_array_if_exists(accessor, "max", a.max);
            doc.accessors.push_back(std::move(a));
        }
    }

    // Parse bufferViews
    if (j.contains("bufferViews")) {
        const auto& bufferViews = j["bufferViews"];
        doc.bufferViews.reserve(bufferViews.size());

        for (const auto& bufferView : bufferViews) {
            GLTFBufferView bv;
            parse_if_exists(bufferView, "buffer", bv.buffer);
            parse_if_exists(bufferView, "byteOffset", bv.byteOffset);
            parse_if_exists(bufferView, "byteLength", bv.byteLength);
            parse_if_exists(bufferView, "byteStride", bv.byteStride);
            parse_if_exists(bufferView, "target", bv.target);
            doc.bufferViews.push_back(std::move(bv));
        }
    }

    // Parse buffers
    if (j.contains("buffers")) {
        const auto& buffers = j["buffers"];
        doc.buffers.reserve(buffers.size());

        for (const auto& buffer : buffers) {
            GLTFBuffer b;
            parse_if_exists(buffer, "byteLength", b.byteLength);
            parse_if_exists(buffer, "uri", b.uri);
            doc.buffers.push_back(std::move(b));
        }
    }

    // Parse meshes
    if (j.contains("meshes")) {
        const auto& meshes = j["meshes"];
        doc.meshes.reserve(meshes.size());

        for (const auto& mesh : meshes) {
            GLTFMesh m;
            parse_if_exists(mesh, "name", m.name);
            parse_array_if_exists(mesh, "weights", m.weights);

            // Parse primitives
            if (mesh.contains("primitives") && mesh["primitives"].is_array()) {
                const auto& primitives = mesh["primitives"];
                m.primitives.reserve(primitives.size());

                for (const auto& primitive : primitives) {
                    GLTFPrimitive p;
                    parse_if_exists(primitive, "indices", p.indices);
                    parse_if_exists(primitive, "material", p.material);
                    parse_if_exists(primitive, "mode", p.mode);

                    // Parse attributes (position, normal, texcoord, etc.)
                    if (primitive.contains("attributes") && primitive["attributes"].is_object()) {
                        const auto& attributes = primitive["attributes"];
                        for (auto it = attributes.begin(); it != attributes.end(); ++it) {
                            p.attributes[it.key()] = it.value().get<int>();
                        }
                    }

                    m.primitives.push_back(std::move(p));
                }
            }

            doc.meshes.push_back(std::move(m));
        }
    }

    // Load the buffer data
    size_t lastSlash = filename.rfind('/');
    std::string baseDir = (lastSlash == std::string::npos) ? "" : filename.substr(0, lastSlash + 1);
    loadBufferData(doc, baseDir);

    // Get default scene
    parse_if_exists(j, "scene", doc.defaultScene);

    return doc;
}
