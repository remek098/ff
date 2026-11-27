#pragma once
#include "EngineAPI.h"
#include <filesystem>

namespace fs = std::filesystem;

namespace project {
struct ProjectEntry {
    std::string name;
    fs::path    full_path;
};

class ProjectBrowser {
  public:
    DISABLE_COPY_AND_MOVE(ProjectBrowser);

    using OnProjectSelected = std::function<void(const std::filesystem::path &selected_game_dir)>;

    static void                  register_imgui_widget(OnProjectSelected on_select = {});
    static std::filesystem::path get_selected_project_dir();

  private:
};
} // namespace project