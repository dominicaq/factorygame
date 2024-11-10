#pragma once
#include "engine.h"

static void fpsCounter(float deltaTime) {
    // FPS calculation
    static float fpsTimer = 0.0f;
    static int frames = 0;

    frames++;
    fpsTimer += deltaTime;
    if (fpsTimer >= 1.0f) {
        float fps = frames / fpsTimer;
        frames = 0;
        fpsTimer = 0.0f;
    }
}
