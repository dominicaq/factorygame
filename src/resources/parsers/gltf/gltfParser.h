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
    std::vector<std::unique_ptr<RawMeshData>>& meshes,
    std::vector<int>& materialIndices);
bool parseBuffers(const json &gltfDoc, const std::string &fileName, std::vector<std::vector<uint8_t>>& buffers);
template <typename T>
void parseAccessorData(
    const json& accessorJson,
    const std::vector<std::vector<uint8_t>>& buffers,
    const glTFBufferView& bufferView,
    std::vector<T>& targetVector);
bool parseNodes(const json& gltfDoc, std::vector<SceneData>& nodeData);
void loadMaterials(const json& gltfDoc,
    std::vector<std::unique_ptr<RawMeshData>>& meshes,
    const std::vector<int>& materialIndices,
    Shader* shader,
    const std::string& fileName);

/*
* Math
*/
void calculateTangentSpace(RawMeshData* mesh, std::vector<glm::vec3>& normals, std::vector<glm::vec4>& tangents);

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

void packTBNframe(RawMeshData* mesh, const std::vector<glm::vec3>& gltfNormals,
             const std::vector<glm::vec4>& gltfTangents) {
    size_t count = std::min(gltfNormals.size(), gltfTangents.size());
    mesh->packedTNBFrame.reserve(mesh->packedTNBFrame.size() + count);

    constexpr int storageSize = 2; // sizeof(int16_t)
    constexpr float bias = 1.0f / ((1 << (storageSize * 8 - 1)) - 1);

    for (size_t i = 0; i < count; ++i) {
        const glm::vec3& n = gltfNormals[i];
        const glm::vec4& t = gltfTangents[i];
        glm::vec3 tangent(t.x, t.y, t.z);

        // Compute cross product: c = n × t (CRITICAL: this is the correct order)
        glm::vec3 c = glm::cross(n, tangent);

        // Construct matrix in COLUMN-MAJOR order: [tangent, c, normal]
        // This matches the JavaScript toMat3 function exactly
        glm::mat3 tbn(
            tangent.x, tangent.y, tangent.z,  // First column: tangent
            c.x,       c.y,       c.z,        // Second column: cross product
            n.x,       n.y,       n.z         // Third column: normal
        );

        // Extract quaternion and normalize
        glm::quat q = glm::normalize(glm::quat_cast(tbn));

        // Ensure positive quaternion (w >= 0)
        if (q.w < 0.0f) {
            q = -q;
        }

        // Apply bias to prevent w from being zero
        if (q.w < bias) {
            q.w = bias;
            float factor = std::sqrt(1.0f - bias * bias);
            q.x *= factor;
            q.y *= factor;
            q.z *= factor;
        }

        // Compute bitangent based on handedness for reflection check
        glm::vec3 b;
        if (t.w > 0.0f) {
            b = glm::cross(tangent, n);
        } else {
            b = glm::cross(n, tangent);
        }

        // Reflection check: if (n × t) · b < 0, negate quaternion
        glm::vec3 cc = glm::cross(tangent, n);
        if (glm::dot(cc, b) < 0.0f) {
            q = -q;
        }

        // Store packed quaternion
        mesh->packedTNBFrame.push_back(glm::vec4(q.x, q.y, q.z, q.w));
    }
}

/*
* Parsing
*/
bool loadglTF(const std::string& fileName, std::vector<std::unique_ptr<RawMeshData>>& meshes, std::vector<SceneData>& nodeData, Shader* shader) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << fileName << "\n";
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    bool isBinary = (fileName.size() >= 3 && fileName.substr(fileName.size() - 3) == "glb");
    if (isBinary) {
        // Binary .glb not supported
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

        glTFBufferView bufferView{0};
        parseJsonProperty(bufferViewJson, "buffer", bufferView.index);
        parseJsonProperty(bufferViewJson, "byteOffset", bufferView.byteOffset);
        parseJsonProperty(bufferViewJson, "byteLength", bufferView.size);
        parseJsonProperty(bufferViewJson, "byteStride", bufferView.stride);

        bufferViews[i] = bufferView;
    }

    // Accessors (primitive mesh data)
    std::vector<int> materialIndices;

    // Note: You will have to update parseAccessors to fill meshes as vector of unique_ptr
    if (!parseAccessors(gltfDoc, buffers, bufferViews, meshes, materialIndices)) {
        std::cerr << "Failed to parse glTF accessors\n";
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
    std::vector<std::unique_ptr<RawMeshData>>& meshes,
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
            auto mesh = std::make_unique<RawMeshData>();
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
            std::vector<glm::vec3> tempNormals;
            std::vector<glm::vec4> tempTangents;

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
                parseAccessorData(normalAccesor, buffers, bufferViews[normalBufferViewIdx], tempNormals);
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

            if (mesh->vertices.empty() || tempNormals.empty() || mesh->indices.empty() || mesh->uvs.empty()) {
                std::stringstream warningMsg;
                warningMsg << "[Warning] failed to load glTF mesh at index [" << primitiveIdx
                        << "]: one or more required attributes are missing.\n"
                        << "Present data: \n"
                        << "    Vertices: " << (mesh->vertices.empty() ? "Missing" : "OK") << "\n"
                        << "    Indices: " << (mesh->indices.empty() ? "Missing" : "OK") << "\n"
                        << "    Normals: " << (tempNormals.empty() ? "Missing" : "OK") << "\n"
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
                parseAccessorData(tangentAccessor, buffers, bufferViews[tangentBufferViewIdx], tempTangents);
            } else {
                std::cerr << "[Warning] Tangents not found for mesh, calculating...\n";
                calculateTangentSpace(mesh.get(), tempNormals, tempTangents);
                std::cout << "[Info] Done.\n";
            }

            // Material index handling
            int materialIndex = -1;
            if (primitiveJson.contains("material")) {
                materialIndex = primitiveJson["material"].get<int>();
            }

            // CRITICAL FIX: Call packTBNframe BEFORE moving the unique_ptr
            // Store the mesh and its corresponding material index
            packTBNframe(mesh.get(), tempNormals, tempTangents);

            // Now it's safe to move the mesh to the container
            meshes.push_back(std::move(mesh));
            materialIndices.push_back(materialIndex);
        }
    }

    return !meshes.empty();
}

void calculateTangentSpace(RawMeshData* mesh, std::vector<glm::vec3>& normals, std::vector<glm::vec4>& tangents) {
    size_t vertexCount = mesh->vertices.size();
    tangents.resize(vertexCount, glm::vec4(0.0f));

    std::vector<glm::vec3> accumulatedTangents(vertexCount, glm::vec3(0.0f));
    std::vector<glm::vec3> accumulatedBitangents(vertexCount, glm::vec3(0.0f));

    for (size_t i = 0; i < mesh->indices.size(); i += 3) {
        uint32_t i0 = mesh->indices[i];
        uint32_t i1 = mesh->indices[i + 1];
        uint32_t i2 = mesh->indices[i + 2];

        if (i0 >= vertexCount || i1 >= vertexCount || i2 >= vertexCount)
            continue;

        glm::vec3 pos0 = mesh->vertices[i0];
        glm::vec3 pos1 = mesh->vertices[i1];
        glm::vec3 pos2 = mesh->vertices[i2];

        glm::vec2 uv0 = mesh->uvs[i0];
        glm::vec2 uv1 = mesh->uvs[i1];
        glm::vec2 uv2 = mesh->uvs[i2];

        glm::vec3 edge1 = pos1 - pos0;
        glm::vec3 edge2 = pos2 - pos0;
        glm::vec2 deltaUV1 = uv1 - uv0;
        glm::vec2 deltaUV2 = uv2 - uv0;

        float det = deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x;
        if (std::abs(det) < 1e-6f)
            continue;

        float invDet = 1.0f / det;

        glm::vec3 tangent = (edge1 * deltaUV2.y - edge2 * deltaUV1.y) * invDet;
        glm::vec3 bitangent = (edge2 * deltaUV1.x - edge1 * deltaUV2.x) * invDet;

        accumulatedTangents[i0] += tangent;
        accumulatedTangents[i1] += tangent;
        accumulatedTangents[i2] += tangent;

        accumulatedBitangents[i0] += bitangent;
        accumulatedBitangents[i1] += bitangent;
        accumulatedBitangents[i2] += bitangent;
    }

    for (size_t i = 0; i < vertexCount; ++i) {
        glm::vec3 n = glm::normalize(normals[i]);  // normalize normal here
        glm::vec3 t = accumulatedTangents[i];
        glm::vec3 b = accumulatedBitangents[i];

        // Orthonormalize the tangent vector using Gram-Schmidt
        t = glm::normalize(t - n * glm::dot(n, t));

        // Recalculate bitangent from orthonormal basis
        glm::vec3 computedB = glm::cross(n, t);

        // Compute handedness: compare computed bitangent with accumulated one
        float w = (glm::dot(computedB, glm::normalize(b)) < 0.0f) ? -1.0f : 1.0f;

        tangents[i] = glm::vec4(t, w);
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

void loadMaterials(const json& gltfDoc, std::vector<std::unique_ptr<RawMeshData>>& meshes,
                   const std::vector<int>& materialIndices, Shader* shader, const std::string& fileName)
{
    if (!gltfDoc.contains("materials") || !gltfDoc["materials"].is_array()) {
        // No materials defined in the glTF file
        for (auto& mesh : meshes) {
            if (mesh) { // Add null check
                Material* newMaterial = new Material(shader);
                mesh->material = newMaterial;
            }
        }
        return;
    }

    const auto& materialsJson = gltfDoc["materials"];

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
        if (!meshes[meshIdx]) { // Add null check
            continue;
        }

        RawMeshData* mesh = meshes[meshIdx].get();
        Material* newMaterial = new Material(shader);
        mesh->material = newMaterial;

        // Add bounds check for materialIndices
        if (meshIdx >= materialIndices.size()) {
            continue;
        }

        int materialIdx = materialIndices[meshIdx];

        if (materialIdx < 0 || materialIdx >= static_cast<int>(materialsJson.size())) {
            continue;
        }

        const auto& materialJson = materialsJson[materialIdx];

        newMaterial->albedoColor = glm::vec4(1.0f);

        if (materialJson.contains("pbrMetallicRoughness")) {
            const auto& pbrJson = materialJson["pbrMetallicRoughness"];

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

            if (pbrJson.contains("baseColorTexture") && pbrJson["baseColorTexture"].contains("index")) {
                size_t textureIdx = pbrJson["baseColorTexture"]["index"].get<size_t>();
                if (textureIdx < textures.size() && textures[textureIdx] != nullptr) {
                    newMaterial->albedoMap = textures[textureIdx];
                }
            }

            if (pbrJson.contains("metallicRoughnessTexture") &&
                pbrJson["metallicRoughnessTexture"].contains("index")) {
                size_t textureIdx = pbrJson["metallicRoughnessTexture"]["index"].get<size_t>();
                if (textureIdx < textures.size() && textures[textureIdx] != nullptr) {
                    newMaterial->metallicRoughnessMap = textures[textureIdx];
                }
            }
        }

        if (materialJson.contains("normalTexture") && materialJson["normalTexture"].contains("index")) {
            size_t textureIdx = materialJson["normalTexture"]["index"].get<size_t>();
            if (textureIdx < textures.size() && textures[textureIdx] != nullptr) {
                newMaterial->normalMap = textures[textureIdx];
            }
        }

        if (materialJson.contains("occlusionTexture") && materialJson["occlusionTexture"].contains("index")) {
            if (materialJson["occlusionTexture"].contains("strength")) {
                newMaterial->occlusionStrength = materialJson["occlusionTexture"]["strength"].get<float>();
            }
        }

        if (materialJson.contains("emissiveTexture") && materialJson["emissiveTexture"].contains("index")) {
            size_t textureIdx = materialJson["emissiveTexture"]["index"].get<size_t>();
            if (textureIdx < textures.size() && textures[textureIdx] != nullptr) {
                newMaterial->emissiveMap = textures[textureIdx];
            }
        }

        if (materialJson.contains("emissiveFactor") && materialJson["emissiveFactor"].is_array() &&
            materialJson["emissiveFactor"].size() == 3) {
            const auto& emissiveArray = materialJson["emissiveFactor"];
            newMaterial->emissiveColor = glm::vec3(
                emissiveArray[0].get<float>(),
                emissiveArray[1].get<float>(),
                emissiveArray[2].get<float>()
            );
        }
    }
}

template <typename T>
void unpackData(const uint8_t* dataPtr, int componentType, int vecSize, T& target) {
    if constexpr (std::is_same_v<T, uint32_t>) {
        switch (componentType) {
            case 5121: { // UNSIGNED_BYTE
                uint8_t value;
                std::memcpy(&value, dataPtr, sizeof(uint8_t));
                target = static_cast<uint32_t>(value);
                break;
            }
            case 5123: { // UNSIGNED_SHORT
                uint16_t value;
                std::memcpy(&value, dataPtr, sizeof(uint16_t));
                target = static_cast<uint32_t>(value);
                break;
            }
            case 5125: { // UNSIGNED_INT
                uint32_t value;
                std::memcpy(&value, dataPtr, sizeof(uint32_t));
                target = value;
                break;
            }
            default:
                std::cerr << "Unsupported component type for indices: " << componentType << "\n";
                break;
        }
        return;
    }

    if constexpr (std::is_same_v<T, glm::vec2> || std::is_same_v<T, glm::vec3> || std::is_same_v<T, glm::vec4>) {
        float values[4] = {0.0f, 0.0f, 0.0f, 1.0f}; // default w = 1.0f

        switch (componentType) {
            case 5120: { // BYTE
                for (int i = 0; i < vecSize; ++i) {
                    int8_t temp;
                    std::memcpy(&temp, dataPtr + i * sizeof(int8_t), sizeof(int8_t));
                    values[i] = static_cast<float>(temp);
                }
                break;
            }
            case 5121: { // UNSIGNED_BYTE
                for (int i = 0; i < vecSize; ++i) {
                    uint8_t temp;
                    std::memcpy(&temp, dataPtr + i * sizeof(uint8_t), sizeof(uint8_t));
                    values[i] = static_cast<float>(temp);
                }
                break;
            }
            case 5122: { // SHORT
                for (int i = 0; i < vecSize; ++i) {
                    int16_t temp;
                    std::memcpy(&temp, dataPtr + i * sizeof(int16_t), sizeof(int16_t));
                    values[i] = static_cast<float>(temp);
                }
                break;
            }
            case 5123: { // UNSIGNED_SHORT
                for (int i = 0; i < vecSize; ++i) {
                    uint16_t temp;
                    std::memcpy(&temp, dataPtr + i * sizeof(uint16_t), sizeof(uint16_t));
                    values[i] = static_cast<float>(temp);
                }
                break;
            }
            case 5125: { // UNSIGNED_INT
                for (int i = 0; i < vecSize; ++i) {
                    uint32_t temp;
                    std::memcpy(&temp, dataPtr + i * sizeof(uint32_t), sizeof(uint32_t));
                    values[i] = static_cast<float>(temp);
                }
                break;
            }
            case 5126: { // FLOAT
                for (int i = 0; i < vecSize; ++i) {
                    float temp;
                    std::memcpy(&temp, dataPtr + i * sizeof(float), sizeof(float));
                    values[i] = temp;
                }
                break;
            }
            default:
                std::cerr << "Unsupported component type: " << componentType << "\n";
                break;
        }

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
