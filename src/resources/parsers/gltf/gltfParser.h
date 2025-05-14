#pragma once

#include "../../../components/mesh.h"
// ... and any other mesh data in the future

#include <json/json.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

using json = nlohmann::json;

// glTF 2.0 Dependancy order
// buffers -> buffer views -> accessors -> other elements.

/*
* Mininmal glTF 2.0 spec (made on a per need basis)
*/
struct glTFBufferView{
    size_t index, byteOFfset, size, stride;
};

/*
* Forward declartation(s)
*/
bool parseAccessors(
    const json& gltfDoc,
    std::vector<std::vector<uint8_t>>& buffers,
    std::vector<glTFBufferView>& bufferViews,
    std::vector<Mesh*>& meshes);
bool parseBuffers(const json &gltfDoc, const std::string &fileName, std::vector<std::vector<uint8_t>>& buffers);
template <typename T>
void parseAccessorData(
    const json& accessorJson,
    const std::vector<std::vector<uint8_t>>& buffers,
    const glTFBufferView& bufferView,
    size_t accessorIndex,
    std::vector<T>& targetVector,
    int vecSize);
bool parseNodes(const json& gltfDoc, std::vector<SceneData>& nodeData);

/*
 * Utility
 */
template<typename T>
void parseJsonProperty(const json& j, const char* key, T& target) {
    auto it = j.find(key);
    if (it != j.end()) {
        target = it->get<T>();
    }
}

std::string base64Decode(const std::string& base64) {
    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::vector<unsigned char> decodedBytes;
    size_t val = 0, valb = -8;

    for (unsigned char c : base64) {
        if (c == '=') break;
        val = (val << 6) + base64_chars.find(c);
        valb += 6;
        if (valb >= 0) {
            decodedBytes.push_back((val >> valb) & 0xFF);
            valb -= 8;
        }
    }

    return std::string(decodedBytes.begin(), decodedBytes.end());
}

size_t getComponentSize(int componentType) {
    switch (componentType) {
        case 5120: return 1; // BYTE
        case 5121: return 1; // UNSIGNED_BYTE
        case 5122: return 2; // SHORT
        case 5123: return 2; // UNSIGNED_SHORT
        case 5126: return 4; // FLOAT
        default: return 0;
    }
}

/*
* Parsing
*/
bool loadglTF(std::string fileName, std::vector<Mesh*>& meshes, std::vector<SceneData>& nodeData) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << fileName << "\n";
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    bool isBinary = (fileName.substr(fileName.size() - 3) == "glb");
    if (isBinary) {
        return false;
    }

    json gltfDoc = json::parse(buffer.str());

    // Preliminary check
    std::string gltfVersion;
    parseJsonProperty(gltfDoc["asset"], "version", gltfVersion);
    if (gltfVersion != "2.0") {
        std::cerr << "Unsupported glTF version: " << gltfVersion << "\n";
        return false;
    }

    // Buffers
    std::vector<std::vector<uint8_t>> buffers;
    if (!parseBuffers(gltfDoc, fileName, buffers)) {
        std::cerr << "Failed to parse glTF buffers\n";
        return false;
    }

    // Buffer views
    if (!gltfDoc.contains("bufferViews")) {
        std::cerr << "Failed to parse glTF bufferViews\n";
        return false;
    }

    std::vector<glTFBufferView> bufferViews;
    const auto& bufferViewsJson = gltfDoc["bufferViews"];
    bufferViews.resize(bufferViewsJson.size());

    for (size_t i = 0; i < bufferViewsJson.size(); ++i) {
        const auto& bufferViewJson = bufferViewsJson[i];

        glTFBufferView bufferView {0};
        parseJsonProperty(bufferViewJson, "buffer", bufferView.index);
        parseJsonProperty(bufferViewJson, "byteOffset", bufferView.byteOFfset);
        parseJsonProperty(bufferViewJson, "byteLength", bufferView.size);
        parseJsonProperty(bufferViewJson, "byteStride", bufferView.stride);

        bufferViews[i] = bufferView;
    }

    // Accessors (primitive mesh data)
    if (!parseAccessors(gltfDoc, buffers, bufferViews, meshes)) {
        std::cerr << "Failed to parse glTF acessors\n";
        return false;
    }

    // Meta data parsing (materials, game object positions, rotations, etc.)
    if (!parseNodes(gltfDoc, nodeData)) {
        std::cerr << "Failed to parse glTF node meta data\n";
        meshes.clear();
        nodeData.clear();
        return false;
    }

    return true;
}

bool parseBuffers(const json& gltfDoc, const std::string& fileName, std::vector<std::vector<uint8_t>>& buffers) {
    if (!gltfDoc.contains("buffers")) {
        return false;
    }

    const auto& buffersJson = gltfDoc["buffers"];
    buffers.resize(buffersJson.size());

    for (size_t i = 0; i < buffersJson.size(); ++i) {
        const auto& bufferJson = buffersJson[i];
        if (!bufferJson.contains("uri")) {
            continue;
        }

        std::string uri;
        parseJsonProperty(bufferJson, "uri", uri);

        if (uri.find("data:application/octet-stream;base64,") == 0) {
            // Handle base64 embedded buffer
            std::string base64Data = uri.substr(37); // Skip the prefix
            std::string decodedData = base64Decode(base64Data);

            buffers[i].resize(decodedData.size());
            std::memcpy(buffers[i].data(), decodedData.data(), decodedData.size());

        } else {
            // Handle external buffer
            std::string bufferPath = fileName.substr(0, fileName.find_last_of("/\\") + 1) + uri;
            std::ifstream bufferFile(bufferPath, std::ios::binary);
            if (!bufferFile.is_open()) {
                std::cerr << "Failed to open buffer file: " << bufferPath << "\n";
                return false;
            }

            size_t byteLength = 0;
            parseJsonProperty(bufferJson, "byteLength", byteLength);
            buffers[i].resize(byteLength);
            bufferFile.read(reinterpret_cast<char*>(buffers[i].data()), byteLength);
        }
    }

    return true;
}

/*
* Accessor parsing
*/
bool parseAccessors(
    const json& gltfDoc,
    std::vector<std::vector<uint8_t>>& buffers,
    std::vector<glTFBufferView>& bufferViews,
    std::vector<Mesh*>& meshes) {

    if (!gltfDoc.contains("accessors") || !gltfDoc["accessors"].is_array() || gltfDoc["accessors"].empty()) {
        std::cerr << "glTF file doesn't contain valid accessors array\n";
        return false;
    }

    if (!gltfDoc.contains("meshes") || !gltfDoc["meshes"].is_array() || gltfDoc["meshes"].empty()) {
        std::cerr << "glTF file doesn't contain valid meshes array\n";
        return false;
    }

    const auto& accessorsJson = gltfDoc["accessors"];
    const auto& meshesJson = gltfDoc["meshes"];

    meshes.clear();
    meshes.reserve(meshesJson.size());

    for (size_t meshIdx = 0; meshIdx < meshesJson.size(); ++meshIdx) {
        const auto& meshJson = meshesJson[meshIdx];
        if (!meshJson.contains("primitives") || !meshJson["primitives"].is_array() || meshJson["primitives"].empty()) {
            std::cerr << "Mesh " << meshIdx << " doesn't contain valid primitives\n";
            continue;
        }

        const auto& primitivesJson = meshJson["primitives"];
        for (size_t primitiveIdx = 0; primitiveIdx < primitivesJson.size(); ++primitiveIdx) {
            Mesh* mesh = new Mesh();
            const auto& primitiveJson = primitivesJson[primitiveIdx];
            if (!primitiveJson.contains("attributes")) {
                continue;
            }

            const auto& attributeJson = primitiveJson["attributes"];

            // Process POSITION attribute
            if (attributeJson.contains("POSITION")) {
                size_t positionIdx = attributeJson["POSITION"];
                const auto& positionAccesor = accessorsJson[positionIdx];
                size_t positionBufferViewIdx = positionAccesor["bufferView"];
                parseAccessorData(positionAccesor, buffers, bufferViews[positionBufferViewIdx], positionIdx, mesh->vertices, 3);
            }

            // Process NORMAL attribute
            if (attributeJson.contains("NORMAL")) {
                size_t normalIdx = attributeJson["NORMAL"];
                const auto& normalAccesor = accessorsJson[normalIdx];
                size_t normalBufferViewIdx = normalAccesor["bufferView"];
                parseAccessorData(normalAccesor, buffers, bufferViews[normalBufferViewIdx], normalIdx, mesh->normals, 3);
            }

            // Process TEXCOORD_0 attribute
            if (attributeJson.contains("TEXCOORD_0")) {
                size_t uvIdx = attributeJson["TEXCOORD_0"];
                const auto& uvAccesor = accessorsJson[uvIdx];
                size_t uvBufferViewIdx = uvAccesor["bufferView"];
                parseAccessorData(uvAccesor, buffers, bufferViews[uvBufferViewIdx], uvIdx, mesh->uvs, 2);
            }

            // Process TANGENT attribute
            if (attributeJson.contains("TANGENT")) {
                size_t tangentIdx = attributeJson["TANGENT"];
                const auto& tangentAccessor = accessorsJson[tangentIdx];
                size_t tangentBufferViewIdx = tangentAccessor["bufferView"];
                parseAccessorData(tangentAccessor, buffers, bufferViews[tangentBufferViewIdx], tangentIdx, mesh->tangents, 3);

                // Calculate bitangents using the tangent vector and normal
                mesh->bitangents.resize(mesh->tangents.size());
                for (size_t i = 0; i < mesh->tangents.size(); ++i) {
                    // Ensure we have normals to calculate bitangent
                    if (i < mesh->normals.size()) {
                        const glm::vec3& normal = mesh->normals[i];
                        const glm::vec3& tangent = mesh->tangents[i];

                        // Calculate bitangent using cross product
                        mesh->bitangents[i] = glm::cross(normal, tangent);
                    }
                }
            }

            // Process Indices
            if (primitiveJson.contains("indices")) {
                size_t indicesAccessorIndex = primitiveJson["indices"];
                const auto& indicesAccessor = accessorsJson[indicesAccessorIndex];
                size_t indicesBufferViewIndex = indicesAccessor["bufferView"];
                parseAccessorData(indicesAccessor, buffers, bufferViews[indicesBufferViewIndex], indicesAccessorIndex, mesh->indices, 1);
            }

            // Material if present
            if (primitiveJson.contains("material")) {
                size_t materialIdx = primitiveJson["material"];
                // TODO: parse material indexes
                // mesh->material = ...
            }

            if (mesh->vertices.empty() || mesh->normals.empty() || mesh->bitangents.empty() || mesh->tangents.empty()) {
                std::stringstream warningMsg;
                warningMsg << "[Warning] Skipping glTF mesh->at index [" << mesh->id
                        << "]: one or more required attributes are missing.\n"
                        << "Present data: \n"
                        << "Vertices: " << (mesh->vertices.empty() ? "Missing" : "OK") << "\n"
                        << "Normals: " << (mesh->normals.empty() ? "Missing" : "OK") << "\n"
                        << "Tangents: " << (mesh->tangents.empty() ? "Missing" : "OK") << "\n"
                        << "Bitangents: " << (mesh->bitangents.empty() ? "Missing" : "OK") << "\n";

                std::cerr << warningMsg.str() << "\n";
                continue;
            }

            meshes.push_back(mesh);
        }
    }

    return !meshes.empty();
}

/*
* Node data parsing
*/
bool parseNodes(const json& gltfDoc, std::vector<SceneData>& nodeData) {
    if (!gltfDoc.contains("nodes") || !gltfDoc["nodes"].is_array()) {
        return false;
    }

    const auto& nodesJson = gltfDoc["nodes"];
    nodeData.resize(nodesJson.size());

    for (size_t i = 0; i < nodesJson.size(); ++i) {
        const auto& nodeJson = nodesJson[i];
        SceneData& node = nodeData[i];

        // Parse name
        if (nodeJson.contains("name")) {
            node.name = nodeJson["name"].get<std::string>();
        }

        // Parse children
        if (nodeJson.contains("children") && nodeJson["children"].is_array()) {
            for (const auto& child : nodeJson["children"]) {
                node.children.push_back(child.get<size_t>());
            }
        }

        // Handle TRS or matrix - matrix takes precedence if both are specified
        if (nodeJson.contains("matrix") && nodeJson["matrix"].is_array()) {
            const auto& matArray = nodeJson["matrix"];
            if (matArray.size() == 16) {
                // Convert column-major 4x4 matrix to our needed format
                // Assuming you're storing this in node.matrix or similar
                // This is a placeholder - you'll want to adapt this based on how you're storing matrices
                std::array<float, 16> matrix;
                for (size_t j = 0; j < 16; ++j) {
                    matrix[j] = matArray[j].get<float>();
                }

                // If you're using glm::mat4 you'd do something like:
                // node.matrix = glm::make_mat4(matrix.data());

                // If you need to extract TRS components from the matrix:
                // glm::vec3 scale, translation;
                // glm::quat rotation;
                // glm::vec3 skew;
                // glm::vec4 perspective;
                // glm::decompose(node.matrix, scale, rotation, translation, skew, perspective);
                // node.position = translation;
                // node.scale = scale;
                // node.eulerAngles = glm::eulerAngles(rotation);
            }
        } else {
            // Parse translation
            node.position = glm::vec3(0.0f);
            if (nodeJson.contains("translation") && nodeJson["translation"].is_array()) {
                const auto& transArray = nodeJson["translation"];
                if (transArray.size() == 3) {
                    for (int j = 0; j < 3; ++j) {
                        node.position[j] = transArray[j].get<float>();
                    }
                }
            }

            // Parse scale
            node.scale = glm::vec3(1.0f);
            if (nodeJson.contains("scale") && nodeJson["scale"].is_array()) {
                const auto& scaleArray = nodeJson["scale"];
                if (scaleArray.size() == 3) {
                    for (int j = 0; j < 3; ++j) {
                        node.scale[j] = scaleArray[j].get<float>();
                    }
                }
            }

            // Parse rotation (quaternion to Euler)
            node.eulerAngles = glm::vec3(0.0f);
            if (nodeJson.contains("rotation") && nodeJson["rotation"].is_array()) {
                const auto& rotArray = nodeJson["rotation"];
                glm::quat q(
                    rotArray[0].get<float>(), // x
                    rotArray[1].get<float>(), // y
                    rotArray[2].get<float>(), // z
                    rotArray[3].get<float>()  // w
                );
                node.eulerAngles = glm::eulerAngles(q);
            }
        }

        // Handle mesh reference if present
        // if (nodeJson.contains("mesh")) {
        //     node.meshIndex = nodeJson["mesh"].get<int>();
        // } else {
        //     node.meshIndex = -1; // No mesh assigned
        // }
    }

    return true;
}

void loadMaterial(const json& gltfDoc, Material& material) {

}

template <typename T>
void unpackData(const uint8_t* dataPtr, int componentType, int vecSize, T& target) {
    // Helper template function for unpacking vector components
    auto unpackVec = [&](auto* typedPtr, float normalizer = 1.0f) {
        if constexpr (std::is_same_v<T, glm::vec2>) {
            target = glm::vec2(
                static_cast<float>(typedPtr[0]) / normalizer,
                static_cast<float>(typedPtr[1]) / normalizer
            );
        } else if constexpr (std::is_same_v<T, glm::vec3>) {
            target = glm::vec3(
                static_cast<float>(typedPtr[0]) / normalizer,
                static_cast<float>(typedPtr[1]) / normalizer,
                static_cast<float>(typedPtr[2]) / normalizer
            );
        } else if constexpr (std::is_same_v<T, glm::vec4>) {
            target = glm::vec4(
                static_cast<float>(typedPtr[0]) / normalizer,
                static_cast<float>(typedPtr[1]) / normalizer,
                static_cast<float>(typedPtr[2]) / normalizer,
                static_cast<float>(typedPtr[3]) / normalizer
            );
        } else if constexpr (std::is_same_v<T, uint32_t>) {
            target = static_cast<uint32_t>(typedPtr[0]);
        }
    };

    // Handle different component types according to glTF 2.0 spec
    switch (componentType) {
        case 5120: // BYTE - signed 8-bit integer
            unpackVec(reinterpret_cast<const int8_t*>(dataPtr), 127.0f);
            break;
        case 5121: // UNSIGNED_BYTE - unsigned 8-bit integer
            unpackVec(dataPtr, 255.0f);
            break;
        case 5122: // SHORT - signed 16-bit integer
            unpackVec(reinterpret_cast<const int16_t*>(dataPtr), 32767.0f);
            break;
        case 5123: // UNSIGNED_SHORT - unsigned 16-bit integer
            unpackVec(reinterpret_cast<const uint16_t*>(dataPtr), 65535.0f);
            break;
        case 5125: // UNSIGNED_INT - unsigned 32-bit integer
            unpackVec(reinterpret_cast<const uint32_t*>(dataPtr));
            break;
        case 5126: // FLOAT - 32-bit float
            unpackVec(reinterpret_cast<const float*>(dataPtr));
            break;
        default:
            std::cerr << "Unsupported component type: " << componentType << "\n";
            break;
    }
}

template <typename T>
void parseAccessorData(
    const json& accessorJson,
    const std::vector<std::vector<uint8_t>>& buffers,
    const glTFBufferView& bufferView,
    size_t accessorIndex,
    std::vector<T>& targetVector,
    int vecSize) {

    size_t byteOffset = 0;
    parseJsonProperty(accessorJson, "byteOffset", byteOffset);

    size_t count = 0;
    parseJsonProperty(accessorJson, "count", count);

    int componentType = 0;
    parseJsonProperty(accessorJson, "componentType", componentType);

    const uint8_t* bufferData = buffers[bufferView.index].data() + bufferView.byteOFfset + byteOffset;
    size_t elementByteSize = getComponentSize(componentType) * vecSize;
    size_t stride = bufferView.stride == 0 ? elementByteSize : bufferView.stride;

    targetVector.resize(count);

    for (size_t i = 0; i < count; ++i) {
        const uint8_t* dataPtr = bufferData + i * stride;
        unpackData(dataPtr, componentType, vecSize, targetVector[i]);
    }
}
