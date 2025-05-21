#pragma once

#include <string>

// Forward declaration
class GameObject;

class Script {
protected:
    GameObject* gameObject = nullptr;

public:
    bool isActive = true;
    void (*updateFunc)(Script*, float) = nullptr;

    virtual ~Script() = default;

    // Virtual functions with default implementations
    virtual void start() {}
    virtual void update(const float& deltaTime) {}

    virtual void onDestroy() {}

    // Protected constructor to prevent direct instantiation
    Script() {
        updateFunc = [](Script* self, float dt) { self->update(dt); };
    }

    // Allow GameObject to set gameObject pointer
    friend class GameObject;
};
