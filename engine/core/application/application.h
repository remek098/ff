#pragma once

#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"

#include "core/logging/logger.h"
#include "utils/ftimer.h"
#include "core/input/input_system.h"
#include "components/components_common.h"
#include "graphics/graphics_context.h"
#include "graphics/swapchain.h"
#include <string_view>
#include <vector>
#include <thread>
#include <atomic>

namespace ff {

enum class Action {
    Quit,
    Jump,
    SaveSpecial,
    Reopen,
    Count
};

struct KeyBinding {
    Action                                  action;
    std::vector<input::KeyboardKeys::Value> required_down;      // e.g., LCTRL, LSHIFT
    input::KeyboardKeys::Value              pressed_this_frame; // e.g., ESC or S
    std::function<void()>                   on_triggered;

    KeyBinding(Action                                  act,
               std::vector<input::KeyboardKeys::Value> mods,
               input::KeyboardKeys::Value              key,
               std::function<void()>                   cb)
        : action(act), required_down(std::move(mods)), pressed_this_frame(key), on_triggered(std::move(cb)) {}
};

struct GameKeyBinding {
    std::string                             name;
    std::vector<input::KeyboardKeys::Value> required_down;
    input::KeyboardKeys::Value              pressed_this_frame;
    std::function<void()>                   on_triggered;
};

struct GameMouseButtonBinding {
    std::string                             name;
    std::vector<input::KeyboardKeys::Value> required_down; // e.g., Hold LSHIFT + Click
    input::MouseButton::Value               button;        // Left, Right, Middle, etc.
    std::function<void()>                   on_triggered;
};

GameKeyBinding         &get_game_key_binding(const std::string &name);
GameMouseButtonBinding &get_game_mouse_button_binding(const std::string &name);

class Application {
  public:
    DISABLE_COPY_AND_MOVE(Application);
    Application() = default;
    Application(const char *name = "ff", u32 width = DEFAULT_WINDOW_WIDTH, u32 height = DEFAULT_WINDOW_HEIGHT);
    virtual ~Application() = default;

    void         run();

    SDL_WindowID get_window_id() { return SDL_GetWindowID(window); }
    SDL_Window  *get_window() { return window; }

    void         register_key_binding(std::string                             action_name,
                                      std::vector<input::KeyboardKeys::Value> modifiers,
                                      input::KeyboardKeys::Value              key,
                                      std::function<void()>                   callback);

    void         register_mouse_binding(std::string                             action_name,
                                        std::vector<input::KeyboardKeys::Value> modifiers,
                                        input::MouseButton::Value               button,
                                        std::function<void()>                   callback);

  protected:
    virtual bool init(const char *name, u32 width, u32 height);
    virtual void update(f32 dt);
    virtual void render();
    virtual void shutdown();

    virtual void handle_input();
    virtual void register_default_input_bindings();

    virtual bool begin_frame();
    virtual void end_frame();

  protected:
    void on_quit_triggered();

  private:
    void       game_thread_proc();
    void       render_thread_proc();

    void       handle_resize();
    VkExtent2D get_window_extent() const;

    void       process_sdl_events(const SDL_Event &event);

  protected:
    SDL_Window *window{ nullptr };
    std::string application_name{ "ff" };
    u32         window_width{ DEFAULT_WINDOW_WIDTH };
    u32         window_height{ DEFAULT_WINDOW_HEIGHT };
    bool        is_running{ false };
    bool        is_minimized{ false };
    bool        swapchain_resize_requested{ false };

    // owned directly and managed by application
    graphics::GraphicsContext gcontext{};
    // graphics::Swapchain       swapchain{};
    // VkSurfaceKHR              surface{ VK_NULL_HANDLE };

    std::vector<KeyBinding> key_bindings;
    std::vector<Action>     triggered_actions_this_frame;

  private:
    utils::FTimer     timer{};

    std::jthread      game_thread;
    std::jthread      render_thread;
    std::atomic<bool> is_game_running{ false };
    // Thread-safe metrics from the game thread to show in the window title
    std::atomic<f32> game_dt{ 0.0f };
    std::atomic<f32> game_fps{ 0.0f };
    std::atomic<f32> render_dt{ 0.0f };
    std::atomic<f32> render_fps{ 0.0f };
};
} // namespace ff