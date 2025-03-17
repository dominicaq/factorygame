#ifndef BOUNDING_VOLUMES_H
#define BOUNDING_VOLUMES_H

#include <glm/glm.hpp>
#include <algorithm>
#include <vector>

class AABB {
private:
    glm::vec3 m_min;
    glm::vec3 m_max;

public:
    // Constructors
    AABB() : m_min(1.0f), m_max(-1.0f) {}
    AABB(const glm::vec3& min, const glm::vec3& max) : m_min(min), m_max(max) {}
    AABB(const std::vector<glm::vec3>& points) {
        if (points.empty()) {
            // Invalid AABB
            m_min = glm::vec3(1.0f);
            m_max = glm::vec3(-1.0f);
            return;
        }

        m_min = points[0];
        m_max = points[0];

        for (const auto& point : points) {
            m_min.x = std::min(m_min.x, point.x);
            m_min.y = std::min(m_min.y, point.y);
            m_min.z = std::min(m_min.z, point.z);

            m_max.x = std::max(m_max.x, point.x);
            m_max.y = std::max(m_max.y, point.y);
            m_max.z = std::max(m_max.z, point.z);
        }
    }

    /*
    * Getters
    */
    const glm::vec3& getMin() const { return m_min; }
    const glm::vec3& getMax() const { return m_max; }

    glm::vec3 getCenter() const {
        return (m_min + m_max) * 0.5f;
    }

    glm::vec3 getExtents() const {
        return (m_max - m_min) * 0.5f;
    }

    glm::vec3 getSize() const {
        return m_max - m_min;
    }

    /*
    * Bounding Box checks
    */
    bool isValid() const {
        return m_min.x <= m_max.x && m_min.y <= m_max.y && m_min.z <= m_max.z;
    }

    // Check if a point is inside the AABB
    bool contains(const glm::vec3& point) const {
        return point.x >= m_min.x && point.x <= m_max.x &&
               point.y >= m_min.y && point.y <= m_max.y &&
               point.z >= m_min.z && point.z <= m_max.z;
    }

    // Check if this AABB intersects with another AABB
    bool intersects(const AABB& other) const {
        return m_min.x <= other.m_max.x && m_max.x >= other.m_min.x &&
               m_min.y <= other.m_max.y && m_max.y >= other.m_min.y &&
               m_min.z <= other.m_max.z && m_max.z >= other.m_min.z;
    }

    /*
    * Bounding box utils
    */
    void expand(const glm::vec3& point) {
        if (!isValid()) {
            m_min = point;
            m_max = point;
            return;
        }

        m_min.x = std::min(m_min.x, point.x);
        m_min.y = std::min(m_min.y, point.y);
        m_min.z = std::min(m_min.z, point.z);

        m_max.x = std::max(m_max.x, point.x);
        m_max.y = std::max(m_max.y, point.y);
        m_max.z = std::max(m_max.z, point.z);
    }

    // Merge this AABB with another, return the new AABB
    AABB merge(const AABB& other) const {
        if (!isValid()) return other;
        if (!other.isValid()) return *this;

        return AABB(
            glm::vec3(
                std::min(m_min.x, other.m_min.x),
                std::min(m_min.y, other.m_min.y),
                std::min(m_min.z, other.m_min.z)
            ),
            glm::vec3(
                std::max(m_max.x, other.m_max.x),
                std::max(m_max.y, other.m_max.y),
                std::max(m_max.z, other.m_max.z)
            )
        );
    }

    // Calculate the closest point on the AABB to a given point
    glm::vec3 closestPoint(const glm::vec3& point) const {
        return glm::clamp(point, m_min, m_max);
    }
};

#endif // BOUNDING_VOLUMES_H
