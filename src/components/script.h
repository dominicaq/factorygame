#ifndef SCRIPT_H
#define SCRIPT_H

#include <string>

// Forward declaration
class GameObject;

class Script {
protected:
    GameObject* gameObject = nullptr;

public:
    bool isActive = true;

    virtual ~Script() = default;

    // Virtual functions with default implementations
    virtual void start() {}
    virtual void update(float deltaTime) {}

    // Protected constructor to prevent direct instantiation
    Script() = default;

    // Allow GameObject to set gameObject pointer
    friend class GameObject;
};

#endif // SCRIPT_H
