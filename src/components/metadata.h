#pragma once

#include <bitset>
#include <string>

struct MetaData {
    std::string name;
    std::bitset<64> layers;
};

struct EntityStatus {
    enum FLAG : std::size_t {
        // Entity life cycle management
        DESTROY_ENTITY,
        // Mesh data
        DIRTY_MODEL_MATRIX,
        // Animations
        IS_BONE,
        ANIMATED,
        FLAG_COUNT
    };

    std::bitset<FLAG_COUNT> status;
};
