#define SDL_MAIN_HANDLED 1
#include "EngineAPI.h"

using namespace ff;

void
save_all_asset_files() {
    FDEBUG("Editor Asset Browser: Save all button pressed.");
}

class Editor : public ff::Application {
  public:
    Editor()
        : ff::Application("FF editor Window", 1920, 1080),
          on_asset_browser_save_all_{ std::move(save_all_asset_files) } {}
    ~Editor() override = default;

  protected:
    bool init(const char *name, u32 width, u32 height) override {
        // 1. Always call the base engine init first so window & Vulkan spin up
        if (!ff::Application::init(name, width, height)) {
            return false;
        }

        // 2. Register game-specific actions using our new dynamic binding system!
        register_key_binding("Player.Jump", {}, input::KeyboardKeys::Value::SPACE, [this]() {
            FINFO("Jump action triggered from game code!");
        });

        register_mouse_binding("Player.Shoot",
                               {}, // Optional modifier: Shift + Left Click
                               input::MouseButton::Value::Left,
                               [this]() {
                                   FINFO("Player clicked Left Mouse Button at ({}, {})",
                                         FInputManager.get_mouse_x(get_window_id()), // Or query coordinates globally
                                         FInputManager.get_mouse_y(get_window_id()));
                               });

        FImGuiLayer.set_dockspace_init_callback([this](ImGuiID dockspace_id, ImVec2 viewport_size) {
            build_default_editor_layout(dockspace_id, viewport_size);
        });
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
        get_registered_editor_elements().push_back(FImGuiLayer.register_element(
            ff::make_uptr<MainMenuBarElement>([this]() { request_reopen(); }, [this]() { request_exit(); })));
        FImGuiLayer.register_element(ff::make_uptr<ff::editor::LambdaEditorElement>([]() {
            ImGui::Begin("Scene Outliner");
            ImGui::Text("World / Entities");
            ImGui::End();
        }));
        FImGuiLayer.register_element(ff::make_uptr<ff::editor::LambdaEditorElement>([this]() {
            ImGui::Begin(
                "Assets browser", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_AlwaysVerticalScrollbar);

            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Save all", "Ctrl + S")) {
                        if (on_asset_browser_save_all_) on_asset_browser_save_all_();
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            // 2. Toolbar strip directly under the menu bar with a "Save all" button
            if (ImGui::SmallButton("Save all")) {
                if (on_asset_browser_save_all_) on_asset_browser_save_all_();
            }
            ImGui::Separator();

            ImGui::Text("Assets");
            ImGui::End();
        }));

        return true;
    }

    void update(f32 dt) override {
        // Call base engine update if needed
        ff::Application::update(dt);

        // Put your game-specific per-frame logic here
    }

  private:
    void build_default_editor_layout(ImGuiID dockspace_id, ImVec2 viewport_size) {
        ImGui::DockBuilderRemoveNode(dockspace_id); // Clear previous layout
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport_size);

        ImGuiID dock_main_id   = dockspace_id;
        ImGuiID dock_right_id  = 0;
        ImGuiID dock_bottom_id = 0;

        // plit off ~13%
        ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.13f, &dock_right_id, &dock_main_id);
        ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.25f, &dock_bottom_id, &dock_main_id);

        // Dock windows into their target nodes (window names must match ImGui::Begin(...) titles)
        ImGui::DockBuilderDockWindow("Scene Outliner", dock_right_id);
        ImGui::DockBuilderDockWindow("Assets browser", dock_bottom_id);

        ImGui::DockBuilderFinish(dockspace_id);
    }

  private:
    std::function<void()> on_asset_browser_save_all_;
};

int
main(int argc, char *argv[]) {
    Editor app;
    app.run();

    return 0;
}