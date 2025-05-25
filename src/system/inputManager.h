#pragma once

#include <GLFW/glfw3.h>
#include <iostream>

class InputManager {
public:
    static const int MAX_KEYS = GLFW_KEY_LAST + 1;  // Track all keys up to GLFW_KEY_LAST

    InputManager() = default;  // Default constructor

    // Initialize InputManager with a window reference
    void init(GLFWwindow* window) {
        m_window = window;
        for (int i = 0; i < MAX_KEYS; ++i) {
            m_keyStates[i] = false;
            m_prevKeyStates[i] = false;
        }
        m_lastX = 0.0;
        m_lastY = 0.0;
    }

    // Update key states by checking GLFW for all keys
    void update() {
        // Copy current key states to previous key states
        for (int key = 0; key < MAX_KEYS; ++key) {
            m_prevKeyStates[key] = m_keyStates[key];
            m_keyStates[key] = glfwGetKey(m_window, key) == GLFW_PRESS;
        }

        // Get mouse position
        glfwGetCursorPos(m_window, &m_mouseX, &m_mouseY);

        if (m_firstMouse) {
            m_lastX = m_mouseX;
            m_lastY = m_mouseY;
            m_firstMouse = false;
        }

        m_xOffset = m_mouseX - m_lastX;
        m_yOffset = m_lastY - m_mouseY;  // Reversed since y-coordinates range from bottom to top

        m_lastX = m_mouseX;
        m_lastY = m_mouseY;

        m_cursorMode = glfwGetInputMode(m_window, GLFW_CURSOR);
    }

    // Check if a specific key is currently held down
    bool isKeyDown(int key) const {
        if (key < 0 || key >= MAX_KEYS) {
            std::cerr << "[Warning] InputManager::isKeyDown(): invalid key: " << key << "\n";
            return false;
        }
        return m_keyStates[key];
    }

    // Check if a specific key was just pressed this frame
    bool isKeyPressed(int key) const {
        if (key < 0 || key >= MAX_KEYS) {
            std::cerr << "[Warning] InputManager::isKeyPressed(): invalid key: " << key << "\n";
            return false;
        }
        return m_keyStates[key] && !m_prevKeyStates[key];
    }

    // Check if a specific key was just released this frame
    bool isKeyReleased(int key) const {
        if (key < 0 || key >= MAX_KEYS) {
            std::cerr << "[Warning] InputManager::isKeyReleased(): invalid key: " << key << "\n";
            return false;
        }
        return !m_keyStates[key] && m_prevKeyStates[key];
    }

    // Set the cursor mode
    void setCursorMode(int mode) {
        glfwSetInputMode(m_window, GLFW_CURSOR, mode);
        m_cursorMode = mode;  // Update the internal state to reflect the change
    }

    // Get mouse X and Y position offsets
    float getMouseXOffset() const {
        return static_cast<float>(m_xOffset);
    }

    float getMouseYOffset() const {
        return static_cast<float>(m_yOffset);
    }

    // Check if the cursor is disabled (for capturing the mouse)
    bool isCursorDisabled() const {
        return m_cursorMode == GLFW_CURSOR_DISABLED;
    }

    int getCursorMode() const {
        return m_cursorMode;
    }

private:
    GLFWwindow* m_window = nullptr;  // Store reference to the window
    bool m_keyStates[MAX_KEYS];  // Track current key states
    bool m_prevKeyStates[MAX_KEYS]; // Track previous key states
    bool m_firstMouse = true;
    double m_lastX, m_lastY, m_mouseX, m_mouseY;
    double m_xOffset, m_yOffset;
    int m_cursorMode;
};
