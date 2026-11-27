#include "project_browser.h"

namespace project {
namespace {

std::string
get_game_projects_directory() {
    std::error_code ec;
    const fs::path  cwd = fs::current_path(ec);
    if (ec) {
        return {};
    }

    const fs::path projects_dir = fs::weakly_canonical(cwd / ".." / ".." / "games", ec);
    if (ec) {
        return (cwd / ".." / ".." / "games").lexically_normal().string();
    }

    return projects_dir.string();
}

std::vector<ProjectEntry>
scan_project_folders(const fs::path &root_dir, std::error_code &ec) {
    std::vector<ProjectEntry> result;
    ec.clear();

    if (root_dir.empty() || !fs::exists(root_dir, ec) || !fs::is_directory(root_dir, ec)) {
        return result;
    }

    for (const auto &entry : fs::directory_iterator(root_dir, fs::directory_options::skip_permission_denied, ec)) {
        if (ec) {
            break;
        }

        std::error_code dir_ec;
        if (entry.is_directory(dir_ec) && !dir_ec) {
            result.push_back(ProjectEntry{ entry.path().filename().string(), entry.path() });
        }
    }

    std::sort(
        result.begin(), result.end(), [](const ProjectEntry &a, const ProjectEntry &b) { return a.name < b.name; });

    return result;
}

// --------------------  global variables --------------------------------
ff::editor::ElementId registered_id;
fs::path              selected_project_dir;
// --------------------  END global variables --------------------------------
} // namespace

void
ProjectBrowser::register_imgui_widget(OnProjectSelected on_select) {
    registered_id = FImGuiLayer.register_element(ff::make_uptr<ff::editor::LambdaEditorElement>([on_select]() {
        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        const ImVec2         center   = viewport->GetCenter();
        const ImVec2         size(640.0f, 420.0f);

        // Use ImGuiCond_Appearing so the user can still drag it after opening,
        // or ImGuiCond_Always if you want it locked to the center every frame.
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(size, ImGuiCond_Appearing);

        const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse;

        // Static state for the lambda; move onto ProjectBrowser if you want per-instance state
        static std::vector<ProjectEntry> projects;
        static int                       selected_index = -1;
        static std::string               status_message;
        static bool                      initialized        = false;

        static bool                      pending_unregister = false;
        fs::path                         project_to_open;
        bool                             should_open  = false;

        auto                             trigger_open = [&](int idx) {
            if (idx >= 0 && idx < static_cast<int>(projects.size())) {
                project_to_open    = projects[idx].full_path;
                should_open        = true;
                pending_unregister = true;
            }
        };

        const fs::path projects_root(get_game_projects_directory());

        auto           refresh_projects = [&]() {
            std::error_code ec;
            projects = scan_project_folders(projects_root, ec);
            if (selected_index >= static_cast<int>(projects.size())) {
                selected_index = -1;
            }
            if (ec) {
                status_message = "Error scanning projects: " + ec.message();
            } else if (projects.empty()) {
                status_message = "No project folders found in: " + projects_root.string();
            } else {
                status_message.clear();
            }
        };

        if (!initialized) {
            refresh_projects();
            initialized = true;
        }

        if (ImGui::Begin("Project browser", nullptr, flags)) {
            ImGui::TextUnformatted(("Projects directory: " + projects_root.string()).c_str());
            ImGui::SameLine();
            if (ImGui::Button("Refresh")) {
                refresh_projects();
            }

            ImGui::Separator();

            if (!status_message.empty()) {
                ImGui::TextWrapped("%s", status_message.c_str());
            }

            // Reserve vertical space for the bottom action bar
            const float footer_height = ImGui::GetFrameHeightWithSpacing() + 8.0f;
            if (ImGui::BeginChild("ProjectListRegion", ImVec2(0.0f, -footer_height), true)) {
                for (int i = 0; i < static_cast<int>(projects.size()); ++i) {
                    const bool is_selected = (selected_index == i);
                    if (ImGui::Selectable(projects[i].name.c_str(), is_selected)) {
                        selected_index = i;
                    }
                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                        selected_index = i;
                        trigger_open(i);
                    }
                }
            }
            ImGui::EndChild();

            ImGui::Separator();

            ImGui::BeginDisabled(selected_index < 0 || selected_index >= static_cast<int>(projects.size()));
            if (ImGui::Button("Open Project")) {
                trigger_open(selected_index);
            }
            ImGui::EndDisabled();
        }
        ImGui::End();

        if (should_open) {
            // Open project_to_open here (or dispatch an event/callback)
            selected_project_dir = project_to_open;
            selected_index       = -1;
            initialized          = false;
            FINFO("Project choosen: {}", project_to_open.string());
            if (on_select) {
                on_select(selected_project_dir);
            }
        }

        if (pending_unregister && registered_id != 0) {
            pending_unregister                       = false;
            const ff::editor::ElementId id_to_remove = registered_id;
            registered_id                            = 0;
            FImGuiLayer.unregister_element(id_to_remove);
        }
    }));
}

fs::path ProjectBrowser::get_selected_project_dir() {
    return selected_project_dir;
}
} // namespace project