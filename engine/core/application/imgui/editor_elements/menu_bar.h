#pragma once

#include "application/imgui/imgui_layer.h"

class MainMenuBarElement : public ff::editor::ImGuiEditorElement {
  public:
    MainMenuBarElement(std::function<void()> on_reopen, std::function<void()> on_exit)
        : on_reopen_(std::move(on_reopen)), on_exit_(std::move(on_exit)) {}

    void update() override {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Reopen", "F11")) {
                    if (on_reopen_) on_reopen_();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit", "Esc")) {
                    if (on_exit_) on_exit_();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

  private:
    std::function<void()> on_reopen_;
    std::function<void()> on_exit_;
};