#pragma once

#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"

#include "components/components_common.h"
#include "core/input/input_system.h"
#include "core/logging/logger.h"
#include "graphics/graphics_context.h"
#include "graphics/swapchain.h"
#include <string_view>
#include <vector>

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

class Application {
  public:
    DISABLE_COPY_AND_MOVE(Application);
    Application() = default;
    Application(const char *name   = "ff",
                u32         width  = DEFAULT_WINDOW_WIDTH,
                u32         height = DEFAULT_WINDOW_HEIGHT);
    virtual ~Application() = default;

    void         run();

    SDL_WindowID get_window_id() { return SDL_GetWindowID(window); }

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

  protected:
    void on_quit_triggered();

  private:
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
    graphics::GraphicsContext context{};
    graphics::Swapchain       swapchain{};
    VkSurfaceKHR              surface{ VK_NULL_HANDLE };

    std::vector<KeyBinding>   key_bindings;
    std::vector<Action>       triggered_actions_this_frame;
};
} // namespace ff