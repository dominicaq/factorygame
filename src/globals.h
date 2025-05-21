// Globals.h
#ifndef GLOBALS_H
#define GLOBALS_H

#include "system/inputManager.h"

// Declare global InputManager and debug variables
extern InputManager inputManager;

struct DebugContext {
    int mode;
    int numDepthSlices;
    bool drawGizmos = true;
};
extern DebugContext DEBUG_CTX;

#endif // GLOBALS_H
