#pragma once

#include "engine.h"

class ViewFrameBuffers : public Script {
public:
    void cycleModes() {
        if (inputManager.isKeyPressed(GLFW_KEY_1)) {
            // Turn off debug mode
            DEBUG_CTX.mode = -1;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_2)) {
            // Position
            DEBUG_CTX.mode = 0;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_3)) {
            // Normal
            DEBUG_CTX.mode = 1;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_4)) {
            // Albedo
            DEBUG_CTX.mode = 2;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_5)) {
            // Depth
            DEBUG_CTX.mode = 3;
        }
        if (inputManager.isKeyPressed(GLFW_KEY_6)) {
            // Depth slices
            DEBUG_CTX.mode = 4;
        }
    }

    void update(float deltaTime) override {
        cycleModes();
    }
};
