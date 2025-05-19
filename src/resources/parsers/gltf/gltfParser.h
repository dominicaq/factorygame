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
    size_t index, byteOffset, size, stride;
};

/*
* Forward declartation(s)
*/
bool parseAccessors(
    const json& gltfDoc,
    std::vector<std::vector<uint8_t>>& buffers,
    std::vector<glTFBufferView>& bufferViews,
    std::vector<Mesh*>& meshes,
    std::vector<int>& materialIndices);
bool parseBuffers(const json &gltfDoc, const std::string &fileName, std::vector<std::vector<uint8_t>>& buffers);
template <typename T>
void parseAccessorData(
    const json& accessorJson,
    const std::vector<std::vector<uint8_t>>& buffers,
    const glTFBufferView& bufferView,
    std::vector<T>& targetVector);
bool parseNodes(const json& gltfDoc, std::vector<SceneData>& nodeData);
void loadMaterials(
    const json& gltfDoc,
    std::vector<Mesh*>& meshes,
    const std::vector<int>& materialIndices,
    Shader* shader,
    const std::string& filePath);

/*
* Math
*/
void calculateTangentSpace(Mesh* mesh);

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
bool loadglTF(std::string fileName, std::vector<Mesh*>& meshes, std::vector<SceneData>& nodeData, Shader* shader) {
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
        parseJsonProperty(bufferViewJson, "byteOffset", bufferView.byteOffset);
        parseJsonProperty(bufferViewJson, "byteLength", bufferView.size);
        parseJsonProperty(bufferViewJson, "byteStride", bufferView.stride);

        bufferViews[i] = bufferView;
    }

    // Accessors (primitive mesh data)
    std::vector<int> materialIndices;
    if (!parseAccessors(gltfDoc, buffers, bufferViews, meshes, materialIndices)) {
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

    loadMaterials(gltfDoc, meshes, materialIndices, shader, fileName);
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
    std::vector<Mesh*>& meshes,
    std::vector<int>& materialIndices) {

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
    materialIndices.reserve(meshesJson.size());

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

            if (primitiveJson.contains("mode")) {
                mesh->drawMode = static_cast<GLenum>(primitiveJson.value("mode", GL_TRIANGLES));
            }

            const auto& attributeJson = primitiveJson["attributes"];

            /*
            * Required attributes
            */

            // Process POSITION attribute
            if (attributeJson.contains("POSITION")) {
                size_t positionIdx = attributeJson["POSITION"];
                const auto& positionAccesor = accessorsJson[positionIdx];
                size_t positionBufferViewIdx = positionAccesor["bufferView"];
                parseAccessorData(positionAccesor, buffers, bufferViews[positionBufferViewIdx], mesh->vertices);
            }

            // Process Indices
            if (primitiveJson.contains("indices")) {
                size_t indicesAccessorIndex = primitiveJson["indices"];
                const auto& indicesAccessor = accessorsJson[indicesAccessorIndex];
                size_t indicesBufferViewIndex = indicesAccessor["bufferView"];
                parseAccessorData(indicesAccessor, buffers, bufferViews[indicesBufferViewIndex], mesh->indices);
            }

            // Process NORMAL attribute
            if (attributeJson.contains("NORMAL")) {
                size_t normalIdx = attributeJson["NORMAL"];
                const auto& normalAccesor = accessorsJson[normalIdx];
                size_t normalBufferViewIdx = normalAccesor["bufferView"];
                parseAccessorData(normalAccesor, buffers, bufferViews[normalBufferViewIdx], mesh->normals);
            }

            // Process TEXCOORD_0 attribute
            if (attributeJson.contains("TEXCOORD_0")) {
                size_t uvIdx = attributeJson["TEXCOORD_0"];
                const auto& uvAccesor = accessorsJson[uvIdx];
                size_t uvBufferViewIdx = uvAccesor["bufferView"];
                parseAccessorData(uvAccesor, buffers, bufferViews[uvBufferViewIdx], mesh->uvs);
                // Flip UVs for engine (glTF uses a coordinate system where (0,0) is the bottom-left, while the engine's top-left is (0,0).)
                for (auto& uv : mesh->uvs) {
                    uv.y = 1.0f - uv.y;
                }
            }

            if (mesh->vertices.empty() || mesh->normals.empty() || mesh->indices.empty() || mesh->uvs.empty()) {
                std::stringstream warningMsg;
                warningMsg << "[Warning] failed to load glTF mesh at index [" << primitiveIdx
                        << "]: one or more required attributes are missing.\n"
                        << "Present data: \n"
                        << "    Vertices: " << (mesh->vertices.empty() ? "Missing" : "OK") << "\n"
                        << "    Indices: " << (mesh->indices.empty() ? "Missing" : "OK") << "\n"
                        << "    Normals: " << (mesh->normals.empty() ? "Missing" : "OK") << "\n"
                        << "    TexCoords: " << (mesh->uvs.empty() ? "Missing" : "OK") << "\n";
                std::cerr << warningMsg.str() << "\n";
                meshes.clear();
                return false;
            }

            /*
            * Optional resources
            */

            // Process TANGENT attribute
            if (attributeJson.contains("TANGENT")) {
                size_t tangentIdx = attributeJson["TANGENT"];
                const auto& tangentAccessor = accessorsJson[tangentIdx];
                size_t tangentBufferViewIdx = tangentAccessor["bufferView"];

                // Temporary vector to hold vec4 tangents from the file
                std::vector<glm::vec4> tempTangents;
                parseAccessorData(tangentAccessor, buffers, bufferViews[tangentBufferViewIdx], tempTangents);

                // Extract xyz components for your storage and keep w for bitangent calculation
                mesh->tangents.resize(tempTangents.size());
                mesh->bitangents.resize(tempTangents.size());

                for (size_t i = 0; i < tempTangents.size(); ++i) {
                    // Extract tangent XYZ for storage
                    mesh->tangents[i] = glm::vec3(tempTangents[i].x, tempTangents[i].y, tempTangents[i].z);

                    // Ensure we have normals to calculate bitangent
                    if (i < mesh->normals.size()) {
                        const glm::vec3& normal = mesh->normals[i];
                        const glm::vec3& tangent = mesh->tangents[i];
                        float handedness = tempTangents[i].w;

                        // Calculate bitangent using cross product and apply handedness
                        mesh->bitangents[i] = handedness * glm::cross(normal, tangent);
                    }
                }
            } else {
                std::cerr << "[Warning] Tangents and BiTangents not found for mesh, calculating...\n";
                calculateTangentSpace(mesh);
                std::cout << "[Info] Done.\n";
            }

            // Material index handling
            int materialIndex = -1;
            if (primitiveJson.contains("material")) {
                materialIndex = primitiveJson["material"].get<int>();
            }

            // Store the mesh and its corresponding material index
            meshes.push_back(mesh);
            materialIndices.push_back(materialIndex);
        }
    }

    return !meshes.empty();
}


void calculateTangentSpace(Mesh* mesh) {
    // Initialize tangents and bitangents
    size_t vertexCount = mesh->vertices.size();
    mesh->tangents.resize(vertexCount, glm::vec3(0.0f));
    mesh->bitangents.resize(vertexCount, glm::vec3(0.0f));

    // Process triangles
    for (size_t i = 0; i < mesh->indices.size(); i += 3) {
        // Get vertex indices for this triangle
        uint32_t i0 = mesh->indices[i];
        uint32_t i1 = mesh->indices[i + 1];
        uint32_t i2 = mesh->indices[i + 2];

        // Skip invalid indices
        if (i0 >= vertexCount || i1 >= vertexCount || i2 >= vertexCount) {
            continue;
        }

        // Get positions and UVs
        glm::vec3 edge1 = mesh->vertices[i1] - mesh->vertices[i0];
        glm::vec3 edge2 = mesh->vertices[i2] - mesh->vertices[i0];
        glm::vec2 deltaUV1 = mesh->uvs[i1] - mesh->uvs[i0];
        glm::vec2 deltaUV2 = mesh->uvs[i2] - mesh->uvs[i0];

        // Calculate determinant
        float det = deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x;
        if (std::abs(det) < 1e-6f) continue; // Skip degenerate triangles

        float invDet = 1.0f / det;

        // Calculate and accumulate tangent/bitangent
        glm::vec3 tangent = (edge1 * deltaUV2.y - edge2 * deltaUV1.y) * invDet;
        glm::vec3 bitangent = (edge2 * deltaUV1.x - edge1 * deltaUV2.x) * invDet;

        // Add to all three vertices of this triangle
        for (uint32_t idx : {i0, i1, i2}) {
            mesh->tangents[idx] += tangent;
            mesh->bitangents[idx] += bitangent;
        }
    }

    // Orthogonalize and normalize
    for (size_t i = 0; i < vertexCount; ++i) {
        const glm::vec3& n = mesh->normals[i];

        // Gram-Schmidt orthogonalization
        glm::vec3& t = mesh->tangents[i];
        t = glm::normalize(t - n * glm::dot(n, t));

        // Handle degenerate tangents
        if (glm::length(t) < 1e-6f) {
            // Find a vector perpendicular to the normal
            t = glm::abs(n.x) > 0.9f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
            t = glm::normalize(t - n * glm::dot(n, t));
        }

        mesh->bitangents[i] = glm::cross(n, t);
    }
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
                // TODO: PLACEHOLDER
                std::array<float, 16> matrix;
                for (size_t j = 0; j < 16; ++j) {
                    matrix[j] = matArray[j].get<float>();
                }
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
        if (nodeJson.contains("mesh")) {
            node.meshIndex = nodeJson["mesh"].get<int>();
        }
    }

    return true;
}

void loadMaterials(const json& gltfDoc, std::vector<Mesh*>& meshes, const std::vector<int>& materialIndices, Shader* shader, const std::string& fileName) {
    if (!gltfDoc.contains("materials") || !gltfDoc["materials"].is_array()) {
        // No materials defined in the glTF file
        for (auto& mesh : meshes) {
            Material* newMaterial = new Material(shader);
            mesh->material = newMaterial;
        }
        return;
    }

    const auto& materialsJson = gltfDoc["materials"];

    // Create vector of textures to store loaded textures
    std::vector<Texture*> textures;
    if (gltfDoc.contains("textures") && gltfDoc["textures"].is_array()) {
        const auto& texturesJson = gltfDoc["textures"];
        const auto& imagesJson = gltfDoc.contains("images") ? gltfDoc["images"] : json();

        textures.resize(texturesJson.size(), nullptr);

        for (size_t i = 0; i < texturesJson.size(); ++i) {
            const auto& textureJson = texturesJson[i];

            if (textureJson.contains("source") && !imagesJson.empty()) {
                size_t imageIndex = textureJson["source"].get<size_t>();

                if (imageIndex < imagesJson.size() && imagesJson[imageIndex].contains("uri")) {
                    std::string imageUri = imagesJson[imageIndex]["uri"].get<std::string>();
                    std::string imgPath = fileName.substr(0, fileName.find_last_of("/\\") + 1) + imageUri;
                    textures[i] = new Texture(imgPath);
                }
            }
        }
    }

    // Now process materials and assign them to meshes
    for (size_t meshIdx = 0; meshIdx < meshes.size(); ++meshIdx) {
        Mesh* mesh = meshes[meshIdx];
        Material* newMaterial = new Material(shader);
        mesh->material = newMaterial;
        int materialIdx = materialIndices[meshIdx];

        if (materialIdx < 0 || materialIdx >= static_cast<int>(materialsJson.size())) {
            continue;
        }

        const auto& materialJson = materialsJson[materialIdx];

        // Default albedo color (if no base color texture is specified)
        newMaterial->albedoColor = glm::vec4(1.0f);

        // Process PBR material properties
        if (materialJson.contains("pbrMetallicRoughness")) {
            const auto& pbrJson = materialJson["pbrMetallicRoughness"];

            // Base color factor (RGBA)
            if (pbrJson.contains("baseColorFactor") && pbrJson["baseColorFactor"].is_array() &&
                pbrJson["baseColorFactor"].size() == 4) {
                const auto& colorArray = pbrJson["baseColorFactor"];
                newMaterial->albedoColor = glm::vec4(
                    colorArray[0].get<float>(),
                    colorArray[1].get<float>(),
                    colorArray[2].get<float>(),
                    colorArray[3].get<float>()
                );
            }

            // Base color texture
            if (pbrJson.contains("baseColorTexture") && pbrJson["baseColorTexture"].contains("index")) {
                size_t textureIdx = pbrJson["baseColorTexture"]["index"].get<size_t>();
                if (textureIdx < textures.size() && textures[textureIdx] != nullptr) {
                    newMaterial->albedoMap = textures[textureIdx];
                }
            }

            // Metallic-roughness texture
            if (pbrJson.contains("metallicRoughnessTexture") &&
                pbrJson["metallicRoughnessTexture"].contains("index")) {
                size_t textureIdx = pbrJson["metallicRoughnessTexture"]["index"].get<size_t>();
                if (textureIdx < textures.size() && textures[textureIdx] != nullptr) {
                    newMaterial->metallicRoughnessMap = textures[textureIdx];
                    // In glTF, metallic and roughness are packed into a single texture
                    // G channel = roughness, B channel = metallic
                    // newMaterial->roughnessMap = textures[textureIdx];
                }
            }
        }

        // Normal map
        if (materialJson.contains("normalTexture") && materialJson["normalTexture"].contains("index")) {
            size_t textureIdx = materialJson["normalTexture"]["index"].get<size_t>();
            if (textureIdx < textures.size() && textures[textureIdx] != nullptr) {
                newMaterial->normalMap = textures[textureIdx];
            }

            // Normal scale factor
            // if (materialJson["normalTexture"].contains("scale")) {
            //     newMaterial->normalScale = materialJson["normalTexture"]["scale"].get<float>();
            // }
        }

        // Occlusion map
        if (materialJson.contains("occlusionTexture") && materialJson["occlusionTexture"].contains("index")) {
            // size_t textureIdx = materialJson["occlusionTexture"]["index"].get<size_t>();
            // if (textureIdx < textures.size() && textures[textureIdx] != nullptr) {
            //     newMaterial->aoMap = textures[textureIdx];
            // }

            // Occlusion strength
            if (materialJson["occlusionTexture"].contains("strength")) {
                newMaterial->occlusionStrength = materialJson["occlusionTexture"]["strength"].get<float>();
            }
        }

        // Emissive map
        if (materialJson.contains("emissiveTexture") && materialJson["emissiveTexture"].contains("index")) {
            size_t textureIdx = materialJson["emissiveTexture"]["index"].get<size_t>();
            if (textureIdx < textures.size() && textures[textureIdx] != nullptr) {
                newMaterial->emissiveMap = textures[textureIdx];
            }
        }

        // Emissive factor
        if (materialJson.contains("emissiveFactor") && materialJson["emissiveFactor"].is_array() &&
            materialJson["emissiveFactor"].size() == 3) {
            const auto& emissiveArray = materialJson["emissiveFactor"];
            newMaterial->emissiveColor = glm::vec3(
                emissiveArray[0].get<float>(),
                emissiveArray[1].get<float>(),
                emissiveArray[2].get<float>()
            );
        }

        // Alpha mode and cutoff
        // if (materialJson.contains("alphaMode")) {
        //     std::string alphaMode = materialJson["alphaMode"].get<std::string>();
        //     if (alphaMode == "MASK") {
        //         material->alphaMode = Material::AlphaMode::MASK;
        //         if (materialJson.contains("alphaCutoff")) {
        //             material->alphaCutoff = materialJson["alphaCutoff"].get<float>();
        //         }
        //     } else if (alphaMode == "BLEND") {
        //         material->alphaMode = Material::AlphaMode::BLEND;
        //     }
        //     // Default is OPAQUE
        // }

        // // Double-sided rendering
        // if (materialJson.contains("doubleSided")) {
        //     material->doubleSided = materialJson["doubleSided"].get<bool>();
        // }
    }
}

template <typename T>
void unpackData(const uint8_t* dataPtr, int componentType, int vecSize, T& target) {
    // Handle scalar values (for indices)
    if constexpr (std::is_same_v<T, uint32_t>) {
        switch (componentType) {
            case 5121: // UNSIGNED_BYTE
                target = static_cast<uint32_t>(*dataPtr);
                break;
            case 5123: // UNSIGNED_SHORT
                target = static_cast<uint32_t>(*reinterpret_cast<const uint16_t*>(dataPtr));
                break;
            case 5125: // UNSIGNED_INT
                target = *reinterpret_cast<const uint32_t*>(dataPtr);
                break;
            default:
                std::cerr << "Unsupported component type for indices: " << componentType << "\n";
                break;
        }
        return;
    }

    // Vector types (vec2, vec3, vec4)
    if constexpr (std::is_same_v<T, glm::vec2> || std::is_same_v<T, glm::vec3> || std::is_same_v<T, glm::vec4>) {
        float values[4] = {0.0f, 0.0f, 0.0f, 1.0f}; // Default w=1.0 for vec4
        float normFactor = 1.0f;
        bool shouldNormalize = false;

        // Extract raw values first
        switch (componentType) {
            case 5120: { // BYTE
                const int8_t* typedPtr = reinterpret_cast<const int8_t*>(dataPtr);
                for (int i = 0; i < vecSize; i++) {
                    values[i] = static_cast<float>(typedPtr[i]);
                }
                normFactor = 127.0f;
                shouldNormalize = true;
                break;
            }
            case 5121: { // UNSIGNED_BYTE
                const uint8_t* typedPtr = dataPtr;
                for (int i = 0; i < vecSize; i++) {
                    values[i] = static_cast<float>(typedPtr[i]);
                }
                normFactor = 255.0f;
                shouldNormalize = true;
                break;
            }
            case 5122: { // SHORT
                const int16_t* typedPtr = reinterpret_cast<const int16_t*>(dataPtr);
                for (int i = 0; i < vecSize; i++) {
                    values[i] = static_cast<float>(typedPtr[i]);
                }
                normFactor = 32767.0f;
                shouldNormalize = true;
                break;
            }
            case 5123: { // UNSIGNED_SHORT
                const uint16_t* typedPtr = reinterpret_cast<const uint16_t*>(dataPtr);
                for (int i = 0; i < vecSize; i++) {
                    values[i] = static_cast<float>(typedPtr[i]);
                }
                normFactor = 65535.0f;
                shouldNormalize = true;
                break;
            }
            case 5125: { // UNSIGNED_INT - no normalization for vertex data
                const uint32_t* typedPtr = reinterpret_cast<const uint32_t*>(dataPtr);
                for (int i = 0; i < vecSize; i++) {
                    values[i] = static_cast<float>(typedPtr[i]);
                }
                break;
            }
            case 5126: { // FLOAT - direct copy
                const float* typedPtr = reinterpret_cast<const float*>(dataPtr);
                for (int i = 0; i < vecSize; i++) {
                    values[i] = typedPtr[i];
                }
                break;
            }
            default:
                std::cerr << "Unsupported component type: " << componentType << "\n";
                break;
        }

        // Apply normalization after extraction (if needed)
        if (shouldNormalize) {
            for (int i = 0; i < vecSize; i++) {
                // Handle signed vs unsigned normalization
                if (componentType == 5120 || componentType == 5122) {
                    // Signed types normalize to [-1, 1]
                    values[i] = std::max(values[i] / normFactor, -1.0f);
                } else {
                    // Unsigned types normalize to [0, 1]
                    values[i] = values[i] / normFactor;
                }
            }
        }

        // Assign the values to the appropriate vector type
        if constexpr (std::is_same_v<T, glm::vec2>) {
            target = glm::vec2(values[0], values[1]);
        } else if constexpr (std::is_same_v<T, glm::vec3>) {
            target = glm::vec3(values[0], values[1], values[2]);
        } else if constexpr (std::is_same_v<T, glm::vec4>) {
            target = glm::vec4(values[0], values[1], values[2], values[3]);
        }
    }
}

template <typename T>
void parseAccessorData(
    const json& accessorJson,
    const std::vector<std::vector<uint8_t>>& buffers,
    const glTFBufferView& bufferView,
    std::vector<T>& targetVector) {

    int vecSize = 0;
    std::string type = accessorJson.value("type", "SCALAR");
    if (type == "SCALAR") {
        vecSize = 1;
    } else if (type == "VEC2") {
        vecSize = 2;
    } else if (type == "VEC3") {
        vecSize = 3;
    } else if (type == "VEC4") {
        vecSize = 4;
    }

    size_t byteOffset = 0;
    parseJsonProperty(accessorJson, "byteOffset", byteOffset);

    size_t count = 0;
    parseJsonProperty(accessorJson, "count", count);

    int componentType = 0;
    parseJsonProperty(accessorJson, "componentType", componentType);

    const uint8_t* bufferData = buffers[bufferView.index].data() + bufferView.byteOffset + byteOffset;
    size_t elementByteSize = getComponentSize(componentType) * vecSize;
    size_t stride = bufferView.stride == 0 ? elementByteSize : bufferView.stride;

    targetVector.resize(count);

    for (size_t i = 0; i < count; ++i) {
        const uint8_t* dataPtr = bufferData + i * stride;
        unpackData(dataPtr, componentType, vecSize, targetVector[i]);
    }
}
