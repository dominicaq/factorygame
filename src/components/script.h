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

    // Virtual functions for derived classes to implement
    virtual void start() = 0;
    virtual void update(float deltaTime) = 0;

    // Protected constructor to prevent direct instantiation
    Script() = default;

    // Allow GameObject to set gameObject pointer
    friend class GameObject;
};

#endif // SCRIPT_H
