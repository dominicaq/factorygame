// void drawEditorLayout(Scene& scene, Renderer& renderer) {
//     // Set up dockspace
//     ImGuiViewport* viewport = ImGui::GetMainViewport();
//     ImGui::SetNextWindowPos(viewport->Pos);
//     ImGui::SetNextWindowSize(viewport->Size);
//     ImGui::SetNextWindowViewport(viewport->ID);
//     ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
//     ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
//     ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
//     windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
//     windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

//     ImGui::Begin("DockSpace", nullptr, windowFlags);
//     ImGui::PopStyleVar(2);

//     // Set up dockspace ID
//     ImGuiID dockspaceId = ImGui::GetID("EditorDockspace");
//     ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

//     // Menu bar
//     if (ImGui::BeginMenuBar()) {
//         if (ImGui::BeginMenu("File")) {
//             if (ImGui::MenuItem("New Scene", "Ctrl+N")) {}
//             if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {}
//             if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {}
//             ImGui::Separator();
//             if (ImGui::MenuItem("Exit", "Alt+F4")) {}
//             ImGui::EndMenu();
//         }
//         if (ImGui::BeginMenu("Edit")) {
//             if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
//             if (ImGui::MenuItem("Redo", "Ctrl+Y")) {}
//             ImGui::EndMenu();
//         }
//         if (ImGui::BeginMenu("View")) {
//             if (ImGui::MenuItem("Reset Layout")) {}
//             ImGui::EndMenu();
//         }
//         ImGui::EndMenuBar();
//     }
//     ImGui::End();

//     // Object Explorer
//     ImGui::Begin("Object Explorer", nullptr, ImGuiWindowFlags_NoCollapse);
//     ImGui::SetWindowPos(ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 2), ImGuiCond_FirstUseEver);
//     ImGui::SetWindowSize(ImVec2(viewport->Size.x * 0.2f, viewport->Size.y - ImGui::GetTextLineHeightWithSpacing() * 2), ImGuiCond_FirstUseEver);

//     if (ImGui::TreeNode("Scene")) {
//         // Display some sample objects
//         if (ImGui::TreeNode("Camera")) {
//             ImGui::Text("Main Camera");
//             ImGui::TreePop();
//         }
//         if (ImGui::TreeNode("Lights")) {
//             ImGui::Selectable("Directional Light");
//             ImGui::Selectable("Point Light 1");
//             ImGui::Selectable("Point Light 2");
//             ImGui::TreePop();
//         }
//         if (ImGui::TreeNode("Meshes")) {
//             static bool selected[3] = { false, false, false };
//             ImGui::Selectable("Cube", &selected[0]);
//             ImGui::Selectable("Floor", &selected[1]);
//             ImGui::Selectable("Character", &selected[2]);
//             ImGui::TreePop();
//         }
//         ImGui::TreePop();
//     }

//     // Add object buttons
//     ImGui::Separator();
//     if (ImGui::Button("Add Object", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
//         ImGui::OpenPopup("AddObjectPopup");
//     }
//     if (ImGui::BeginPopup("AddObjectPopup")) {
//         if (ImGui::MenuItem("Empty Entity")) {}
//         if (ImGui::MenuItem("Cube")) {}
//         if (ImGui::MenuItem("Sphere")) {}
//         if (ImGui::MenuItem("Light")) {}
//         if (ImGui::MenuItem("Camera")) {}
//         ImGui::EndPopup();
//     }

//     ImGui::End();

//     // Game View (Viewport)
//     ImGui::Begin("Game View", nullptr, ImGuiWindowFlags_NoCollapse);
//     ImGui::SetWindowPos(ImVec2(viewport->Size.x * 0.2f, ImGui::GetTextLineHeightWithSpacing() * 2), ImGuiCond_FirstUseEver);
//     ImGui::SetWindowSize(ImVec2(viewport->Size.x * 0.6f, viewport->Size.y - ImGui::GetTextLineHeightWithSpacing() * 2), ImGuiCond_FirstUseEver);

//     // Toolbar above game view
//     ImGui::Button("Play", ImVec2(50, 0));
//     ImGui::SameLine();
//     ImGui::Button("Pause", ImVec2(50, 0));
//     ImGui::SameLine();
//     ImGui::Button("Stop", ImVec2(50, 0));
//     ImGui::SameLine();
//     ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 180);
//     ImGui::Text("FPS: 60");
//     ImGui::SameLine();

//     // Game viewport (placeholder for the actual render)
//     ImVec2 viewportSize = ImGui::GetContentRegionAvail();
//     ImGui::Image((void*)(intptr_t)renderer.getFinalColorBufferID(), viewportSize,
//                  ImVec2(0, 1), ImVec2(1, 0));  // Assuming renderer has this method

//     ImGui::End();

//     // Properties Window
//     ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_NoCollapse);
//     ImGui::SetWindowPos(ImVec2(viewport->Size.x * 0.8f, ImGui::GetTextLineHeightWithSpacing() * 2), ImGuiCond_FirstUseEver);
//     ImGui::SetWindowSize(ImVec2(viewport->Size.x * 0.2f, viewport->Size.y - ImGui::GetTextLineHeightWithSpacing() * 2), ImGuiCond_FirstUseEver);

//     // Display properties based on selection
//     ImGui::Text("Selected: Cube");
//     ImGui::Separator();

//     if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
//         ImGui::Text("Position");
//         float pos[3] = { 0.0f, 0.0f, 0.0f };
//         ImGui::DragFloat3("##Position", pos, 0.1f);

//         ImGui::Text("Rotation");
//         float rot[3] = { 0.0f, 0.0f, 0.0f };
//         ImGui::DragFloat3("##Rotation", rot, 0.1f);

//         ImGui::Text("Scale");
//         float scale[3] = { 1.0f, 1.0f, 1.0f };
//         ImGui::DragFloat3("##Scale", scale, 0.1f);
//     }

//     if (ImGui::CollapsingHeader("Mesh Renderer", ImGuiTreeNodeFlags_DefaultOpen)) {
//         const char* meshTypes[] = { "Cube", "Sphere", "Custom" };
//         static int currentMeshType = 0;
//         ImGui::Combo("Mesh Type", &currentMeshType, meshTypes, IM_ARRAYSIZE(meshTypes));

//         ImGui::Text("Material");
//         static float color[4] = { 0.8f, 0.3f, 0.2f, 1.0f };
//         ImGui::ColorEdit4("Color", color);

//         static float metallic = 0.5f;
//         ImGui::SliderFloat("Metallic", &metallic, 0.0f, 1.0f);

//         static float roughness = 0.3f;
//         ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f);
//     }

//     if (ImGui::CollapsingHeader("Physics")) {
//         static bool useGravity = true;
//         ImGui::Checkbox("Use Gravity", &useGravity);

//         static float mass = 1.0f;
//         ImGui::DragFloat("Mass", &mass, 0.1f, 0.0f, 1000.0f);

//         static bool isKinematic = false;
//         ImGui::Checkbox("Is Kinematic", &isKinematic);
//     }

//     ImGui::End();
// }
