#ifndef OCTREE_H
#define OCTREE_H

#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include "../math/bounding_volumes.h"

// Configuration constants
#define OCT_MAX_DEPTH 10
#define OCT_MIN_DEPTH 3
#define OCT_CELL_CAPACITY 16
#define OCT_ADAPTIVE_THRESHOLD 32

template <typename T>
class Octree {
public:
    // Cell structure for adaptive subdivision
    struct Cell {
        std::vector<T> objects;
        uint8_t depth;
        bool subdivided;
    };

    // Constructor with world bounds
    Octree(const AABB& worldBounds);
    ~Octree() = default;

    // Core functionality
    void insert(const T& object, const glm::vec3& position);
    bool remove(const T& object, const glm::vec3& position);
    std::vector<T> query(const glm::vec3& point) const;
    std::vector<T> query(const AABB& bounds) const;

    // Fast indexing methods
    uint64_t getPointIndex(const glm::vec3& point, uint8_t depth = OCT_MIN_DEPTH) const;
    uint64_t getAdaptiveIndex(const glm::vec3& point) const;

    // Statistics and debug methods
    size_t getObjectCount() const;
    size_t getCellCount() const;
    float getAverageObjectsPerCell() const;
    uint8_t getMaxUsedDepth() const;

private:
    // Adaptive spatial mapping
    std::unordered_map<uint64_t, Cell> cellMap;
    AABB worldBounds;
    size_t objectCount;
    uint8_t maxUsedDepth;

    // Helper methods
    void subdivideCell(uint64_t cellIndex);
    void mergeCell(uint64_t cellIndex);
    std::vector<uint64_t> getChildIndices(uint64_t parentIndex) const;
    uint64_t getParentIndex(uint64_t childIndex) const;
    glm::vec3 getCellCenter(uint64_t index) const;
    float getCellSize(uint8_t depth) const;
    uint8_t getDepthFromIndex(uint64_t index) const;
    std::vector<uint64_t> getCellsInBounds(const AABB& bounds) const;
    bool shouldSubdivide(const Cell& cell) const;
    bool shouldMerge(uint64_t parentIndex) const;
};

#endif // OCTREE_H
