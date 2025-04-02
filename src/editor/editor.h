#pragma once

#include "../engine.h"
#include "../scene/scene.h"
#include "imgui/imgui.h"
#include "terminal.h"

class Editor {
public:
    Editor(int screenWidth, int screenHeight) : m_saveFrameBuffer(screenWidth, screenHeight, 1, false) {
        m_width = screenWidth;
        m_height = screenHeight;

        // Default size ratios
        m_verticalSplitRatio = 0.6f;  // Top/Bottom split for left and center panels
        m_leftPanelRatio = 0.2f;      // Left panel width ratio
        m_rightPanelRatio = 0.2f;     // Right panel width ratio
        m_renderer = nullptr;         // Initialize renderer pointer

        Terminal& terminal = Terminal::getInstance();
        terminal.registerStdCapture();
    }

    ~Editor() {
        Terminal& terminal = Terminal::getInstance();
        terminal.restoreStdCapture();
    }

    // Set renderer reference
    void setRenderer(Renderer* renderer) {
        m_renderer = renderer;
    }

    void drawEditorLayout(Scene& scene, Renderer& renderer) {
        // Store reference to renderer if not already set
        if (m_renderer == nullptr) {
            m_renderer = &renderer;
        }

        copyAndClearScreen();

        // Get current window dimensions to handle resizing
        ImGuiViewport* viewport = ImGui::GetMainViewport();

        // Check if window size has changed
        if (m_width != viewport->Size.x || m_height != viewport->Size.y) {
            m_width = (int)viewport->Size.x;
            m_height = (int)viewport->Size.y;

            // Resize framebuffer and renderer
            m_saveFrameBuffer.resize(m_width, m_height);
            if (m_renderer != nullptr) {
                m_renderer->resize(m_width, m_height);
            }
        }

        // Set up dockspace with fixed layout
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGuiWindowFlags mainWindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar |
                                      ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                      ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                      ImGuiWindowFlags_NoNavFocus;

        ImGui::Begin("DockSpace", nullptr, mainWindowFlags);
        ImGui::PopStyleVar(2);

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New Scene", "Ctrl+N")) {}
                if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {}
                if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {}
                ImGui::Separator();
                if (ImGui::MenuItem("Exit", "Alt+F4")) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
                if (ImGui::MenuItem("Redo", "Ctrl+Y")) {}
                ImGui::Separator();
                if (ImGui::MenuItem("Duplicate", "Ctrl+D")) {}
                if (ImGui::MenuItem("Delete", "Del")) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                if (ImGui::MenuItem("Game", "Ctrl+1")) {}
                if (ImGui::MenuItem("Scene", "Ctrl+2")) {}
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // Calculate the sizes based on the current window size and split ratios
        const float menuBarHeight = ImGui::GetFrameHeight();
        const float terminalHeight = m_height * (1.0f - m_verticalSplitRatio) - menuBarHeight;
        const float topSectionHeight = m_height * m_verticalSplitRatio - menuBarHeight;
        const float fullHeight = m_height - menuBarHeight;

        // Fix the right panel clipping by ensuring panel ratios sum to 1.0
        // and adjusting for padding/borders
        const float totalAvailableWidth = m_width - 16.0f; // Account for window padding
        float totalPanelRatio = m_leftPanelRatio + m_rightPanelRatio;

        // If initial sum of ratios is too large, scale them proportionally
        if (totalPanelRatio > 0.7f) {
            float scale = 0.7f / totalPanelRatio;
            m_leftPanelRatio *= scale;
            m_rightPanelRatio *= scale;
            totalPanelRatio = m_leftPanelRatio + m_rightPanelRatio;
        }

        // Calculate the center ratio
        float centerRatio = 1.0f - totalPanelRatio;

        // Ensure minimum sizes for all panels
        if (centerRatio < 0.3f) {
            centerRatio = 0.3f;
            // Adjust side panels proportionally
            if (totalPanelRatio > 0) {
                float scale = (1.0f - centerRatio) / totalPanelRatio;
                m_leftPanelRatio *= scale;
                m_rightPanelRatio *= scale;
            } else {
                m_leftPanelRatio = m_rightPanelRatio = (1.0f - centerRatio) / 2.0f;
            }
        }

        // Calculate actual pixel widths
        const float leftPanelWidth = totalAvailableWidth * m_leftPanelRatio;
        const float rightPanelWidth = totalAvailableWidth * m_rightPanelRatio;
        const float centerPanelWidth = totalAvailableWidth * centerRatio;

        // Account for the splitter widths
        const float splitterWidth = 8.0f;
        const float adjustedLeftWidth = leftPanelWidth - (splitterWidth / 2);
        const float adjustedCenterWidth = centerPanelWidth - splitterWidth;
        const float adjustedRightWidth = rightPanelWidth - (splitterWidth / 2);

        // Start the main layout with left/center sections and terminal at bottom
        ImGui::BeginChild("MainLayout", ImVec2(m_width - adjustedRightWidth - splitterWidth, 0), false);

        // Top section containing hierarchy and game view
        ImGui::BeginChild("TopSection", ImVec2(0, topSectionHeight), false);

        // Left panel - Scene Hierarchy
        ImGui::BeginChild("LeftPanel", ImVec2(adjustedLeftWidth, -1), true);
        drawSceneHierarchy();
        ImGui::EndChild();

        // Left panel resizer
        ImGui::SameLine();
        if (ImGui::InvisibleButton("left_hsplitter", ImVec2(splitterWidth, topSectionHeight)))
            {}
        if (ImGui::IsItemActive())
        {
            float delta = ImGui::GetIO().MouseDelta.x;
            m_leftPanelRatio += delta / totalAvailableWidth;

            // Ensure minimum sizes
            if (m_leftPanelRatio < 0.1f) m_leftPanelRatio = 0.1f;
            if (m_leftPanelRatio > 0.4f) m_leftPanelRatio = 0.4f;

            // Re-calculate center ratio to maintain constraints
            centerRatio = 1.0f - m_leftPanelRatio - m_rightPanelRatio;
            if (centerRatio < 0.3f) {
                centerRatio = 0.3f;
                m_rightPanelRatio = 1.0f - m_leftPanelRatio - centerRatio;
                if (m_rightPanelRatio < 0.1f) {
                    m_rightPanelRatio = 0.1f;
                    m_leftPanelRatio = 1.0f - m_rightPanelRatio - centerRatio;
                }
            }
        }
        ImGui::SetItemAllowOverlap();
        if (ImGui::IsItemHovered())
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

        // Center panel - Game View
        ImGui::SameLine();
        ImGui::BeginChild("CenterPanel", ImVec2(0, -1), true);
        drawGame();
        ImGui::EndChild();

        ImGui::EndChild(); // End TopSection

        // Vertical resizer
        float dummy_y = topSectionHeight;
        if (ImGui::InvisibleButton("vsplitter", ImVec2(m_width - adjustedRightWidth - splitterWidth, splitterWidth)))
            {}
        if (ImGui::IsItemActive())
        {
            dummy_y += ImGui::GetIO().MouseDelta.y;
            m_verticalSplitRatio = (dummy_y + menuBarHeight) / m_height;
            // Clamp to reasonable values
            if (m_verticalSplitRatio < 0.5f) m_verticalSplitRatio = 0.5f;
            if (m_verticalSplitRatio > 0.9f) m_verticalSplitRatio = 0.9f;
        }
        ImGui::SetItemAllowOverlap();
        if (ImGui::IsItemHovered())
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);

        // Bottom panel - Terminal
        ImGui::BeginChild("BottomPanel", ImVec2(0, 0), true);
        Terminal::getInstance().drawTerminal();
        ImGui::EndChild();

        ImGui::EndChild(); // End MainLayout

        // Main vertical splitter for right panel
        ImGui::SameLine();
        if (ImGui::InvisibleButton("main_vsplitter", ImVec2(splitterWidth, fullHeight)))
            {}
        if (ImGui::IsItemActive())
        {
            float delta = ImGui::GetIO().MouseDelta.x;
            m_rightPanelRatio -= delta / totalAvailableWidth;

            // Ensure minimum sizes
            if (m_rightPanelRatio < 0.1f) m_rightPanelRatio = 0.1f;
            if (m_rightPanelRatio > 0.4f) m_rightPanelRatio = 0.4f;

            // Re-calculate center ratio to maintain constraints
            centerRatio = 1.0f - m_leftPanelRatio - m_rightPanelRatio;
            if (centerRatio < 0.3f) {
                centerRatio = 0.3f;
                m_leftPanelRatio = 1.0f - m_rightPanelRatio - centerRatio;
                if (m_leftPanelRatio < 0.1f) {
                    m_leftPanelRatio = 0.1f;
                    m_rightPanelRatio = 1.0f - m_leftPanelRatio - centerRatio;
                }
            }
        }
        ImGui::SetItemAllowOverlap();
        if (ImGui::IsItemHovered())
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

        // Right panel - Property View (Full Height)
        ImGui::SameLine();
        ImGui::BeginChild("RightPanel", ImVec2(0, 0), true);
        drawPropertyView();
        ImGui::EndChild();

        ImGui::End(); // End main window
    }

private:
    Framebuffer m_saveFrameBuffer;
    int m_width, m_height;
    float m_verticalSplitRatio;
    float m_leftPanelRatio;
    float m_rightPanelRatio;
    Renderer* m_renderer;

    // Property view variables
    static float m_entityProperty;

    // Terminal variables
    static char m_terminalInput[256];
    static bool m_terminalFocus;

    void drawSceneHierarchy() {
        ImGui::Text("Scene Hierarchy");
        ImGui::Separator();

        // Add a search bar for filtering objects
        static char searchBuffer[64] = "";
        ImGui::InputText("Search", searchBuffer, IM_ARRAYSIZE(searchBuffer));
        ImGui::Separator();

        // Context menu for right-click operations
        if (ImGui::BeginPopupContextWindow("HierarchyContextMenu")) {
            if (ImGui::MenuItem("Create Empty")) {}
            if (ImGui::MenuItem("Create 3D Object")) {}
            if (ImGui::BeginMenu("Create Light")) {
                if (ImGui::MenuItem("Directional Light")) {}
                if (ImGui::MenuItem("Point Light")) {}
                if (ImGui::MenuItem("Spot Light")) {}
                ImGui::EndMenu();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Rename")) {}
            if (ImGui::MenuItem("Duplicate")) {}
            if (ImGui::MenuItem("Delete")) {}
            ImGui::EndPopup();
        }
    }

    void drawGame() {
        ImGui::Text("Game View");
        ImGui::Separator();

        // Calculate available width to position the 16:9 button at the far right
        float windowWidth = ImGui::GetContentRegionAvail().x;
        float rightSideWidth = 120.0f; // Width for the 16:9 button and text

        // Toolbar for game controls
        if (ImGui::Button("Play")) {}
        ImGui::SameLine();
        if (ImGui::Button("Pause")) {}
        ImGui::SameLine();
        if (ImGui::Button("Stop")) {}
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100);
        static float gameSpeed = 1.0f;
        ImGui::SliderFloat("Speed", &gameSpeed, 0.1f, 2.0f);

        // Push the 16:9 toggle to the far right
        ImGui::SameLine();
        float spacing = windowWidth - ImGui::GetCursorPosX() - rightSideWidth;
        if (spacing > 0)
            ImGui::Dummy(ImVec2(spacing, 0.0f));

        // Add 16:9 aspect ratio toggle button on the same line
        ImGui::SameLine();
        static bool enforce16by9 = false;
        if (ImGui::Button(enforce16by9 ? "16:9 On" : "16:9 Off")) {
            enforce16by9 = !enforce16by9;
        }

        // Get content region for rendering
        ImVec2 contentSize = ImGui::GetContentRegionAvail();
        ImVec2 renderSize = contentSize;
        float offsetX = 0.0f, offsetY = 0.0f;

        // Apply aspect ratio if enabled
        if (enforce16by9) {
            float aspectRatio = 16.0f / 9.0f;
            if (contentSize.x / contentSize.y > aspectRatio) {
                renderSize.x = contentSize.y * aspectRatio;
                offsetX = (contentSize.x - renderSize.x) * 0.5f;
            } else {
                renderSize.y = contentSize.x / aspectRatio;
                offsetY = (contentSize.y - renderSize.y) * 0.5f;
            }
        }

        // Set cursor position to account for centering when using 16:9
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY);

        // Render the texture
        ImTextureID textureID = (ImTextureID)(intptr_t)m_saveFrameBuffer.getColorAttachment(0);
        ImGui::Image(textureID, renderSize, ImVec2(0, 1), ImVec2(1, 0));

        // Position resolution text at the top right of the game view
        std::string resText = "Resolution: " + std::to_string((int)renderSize.x) + "x" + std::to_string((int)renderSize.y);
        ImVec2 textSize = ImGui::CalcTextSize(resText.c_str());
        ImVec2 textPos = ImGui::GetItemRectMin();
        textPos.x += renderSize.x - textSize.x - 10; // 10 pixels padding from right edge
        textPos.y += 10; // 10 pixels padding from top

        ImGui::GetWindowDrawList()->AddText(textPos, IM_COL32(255, 255, 255, 255), resText.c_str());
    }

    void drawPropertyView() {
        ImGui::Text("Properties");
        ImGui::Separator();

        // Entity selection dummy dropdown
        static int selectedEntity = 0;
        const char* entities[] = { "Player", "Enemy", "Camera", "Light" };
        ImGui::Combo("Entity", &selectedEntity, entities, IM_ARRAYSIZE(entities));

        ImGui::Spacing();
        ImGui::Text("Transform");

        // Basic transform properties
        static float position[3] = { 0.0f, 0.0f, 0.0f };
        static float rotation[3] = { 0.0f, 0.0f, 0.0f };
        static float scale[3] = { 1.0f, 1.0f, 1.0f };

        ImGui::DragFloat3("Position", position, 0.1f);
        ImGui::DragFloat3("Rotation", rotation, 0.1f);
        ImGui::DragFloat3("Scale", scale, 0.1f);

        ImGui::Spacing();
        ImGui::Text("Properties");

        // Entity-specific property
        ImGui::SliderFloat("Entity Property", &m_entityProperty, 0.0f, 100.0f);

        // Add component button
        if (ImGui::Button("Add Component", ImVec2(-1, 0))) {
            ImGui::OpenPopup("AddComponentPopup");
        }

        if (ImGui::BeginPopup("AddComponentPopup")) {
            if (ImGui::MenuItem("Renderer")) {}
            if (ImGui::MenuItem("Rigidbody")) {}
            if (ImGui::MenuItem("Collider")) {}
            if (ImGui::MenuItem("Audio Source")) {}
            if (ImGui::MenuItem("Light")) {}
            if (ImGui::MenuItem("Camera")) {}
            ImGui::EndPopup();
        }
    }

    void copyAndClearScreen() {
        // Retrieve the current viewport dimensions
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        GLint x = viewport[0];
        GLint y = viewport[1];
        GLsizei width = viewport[2];
        GLsizei height = viewport[3];

        // Bind the framebuffer to save the current screen content
        m_saveFrameBuffer.bind();
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

        glBlitFramebuffer(x, y, x + width, y + height,
                          0, 0, width, height,
                          GL_COLOR_BUFFER_BIT, GL_NEAREST);

        // Unbind the framebuffer and return to default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
};

// Initialize static member variables
float Editor::m_entityProperty = 50.0f;
char Editor::m_terminalInput[256] = {0};
bool Editor::m_terminalFocus = false;
