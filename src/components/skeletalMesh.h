#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <entt/entt.hpp>

#include "mesh.h"

#define MAX_BONE_INFLUENCE 4

struct Bone {
    glm::mat4 offsetMatrix;
    std::string name;
    entt::entity entity;
    int index;
};

struct VertexBoneData {
    int boneIDs[MAX_BONE_INFLUENCE] = {-1, -1, -1, -1};
    float weights[MAX_BONE_INFLUENCE] = {0.0f, 0.0f, 0.0f, 0.0f};

    // Add bone influence to this vertex
    void addBoneData(int boneID, float weight) {
        // Find an empty slot
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
            if (boneIDs[i] < 0) {
                boneIDs[i] = boneID;
                weights[i] = weight;
                return;
            }
        }

        // If no empty slot, find the smallest weight
        int minIndex = 0;
        for (int i = 1; i < MAX_BONE_INFLUENCE; i++) {
            if (weights[i] < weights[minIndex]) {
                minIndex = i;
            }
        }

        // Replace if new weight is larger
        if (weight > weights[minIndex]) {
            boneIDs[minIndex] = boneID;
            weights[minIndex] = weight;
        }
    }

    // Normalize weights to sum to 1.0
    void normalizeWeights() {
        float sum = 0.0f;
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
            if (boneIDs[i] >= 0) {
                sum += weights[i];
            }
        }

        if (sum > 0.0f) {
            for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
                if (boneIDs[i] >= 0) {
                    weights[i] /= sum;
                }
            }
        }
    }
};

// Animation keyframe for a single bone
struct BoneKeyframe {
    float timeStamp;
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
};

// Animation track for a single bone
struct BoneAnimationTrack {
    std::string boneName;
    std::vector<BoneKeyframe> keyframes;
};

// Full animation containing tracks for multiple bones
struct Animation {
    std::string name;
    float duration;
    std::vector<BoneAnimationTrack> tracks;
    std::unordered_map<std::string, size_t> trackMap; // Maps bone name to track index
};

// Component for skeleton animation state
struct SkeletonAnimator {
    std::vector<glm::mat4> finalBoneMatrices; // Bone transforms for shader
    int currentAnimation = -1;
    float currentTime = 0.0f;
    float playbackSpeed = 1.0f;
    bool playing = false;
    bool loop = true;
    entt::entity rootBone = entt::null;
};

struct SkeletalMesh : Mesh {
    // Bone hierarchy and skinning data
    std::vector<VertexBoneData> vertexBoneData;           // Per-vertex bone influences
    std::unordered_map<std::string, int> boneNameToIndex; // Maps bone names to indices
    std::vector<Animation> animations;                    // Available animations
    std::vector<glm::mat4> inverseBindPoses;              // Inverse bind matrices for each bone

    // Clear skeletal data
    void clearSkeletalData() {
        Mesh::clearData();
        vertexBoneData.clear();
        boneNameToIndex.clear();
        animations.clear();
        inverseBindPoses.clear();

        vertexBoneData.shrink_to_fit();
        animations.shrink_to_fit();
        inverseBindPoses.shrink_to_fit();
    }

    // Find bone by name
    int findBoneByName(const std::string& name) const {
        auto it = boneNameToIndex.find(name);
        if (it != boneNameToIndex.end()) {
            return it->second;
        }
        return -1;
    }

    // Get animation by name
    int findAnimationByName(const std::string& name) const {
        for (size_t i = 0; i < animations.size(); i++) {
            if (animations[i].name == name) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }
};
