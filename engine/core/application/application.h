#pragma once

#include "defines.h"
#include "graphics/graphics_context.h"
#include "graphics/swapchain.h"
#include <string_view>

struct SDL_Window;

namespace ff {

class Application {
  public:
    DISABLE_COPY_AND_MOVE(Application);
    Application() = default;
    Application(const char* name = "ff", u32 width = DEFAULT_WINDOW_WIDTH, u32 height = DEFAULT_WINDOW_HEIGHT);
    virtual ~Application() = default;

    void run();

  protected:
    virtual bool init(const char* name, u32 width, u32 height);
    virtual void update(f32 dt);
    virtual void render();
    virtual void shutdown();

    virtual void handle_input();
  private:
    void handle_resize();
    VkExtent2D get_window_extent() const;

  protected:
    SDL_Window *window{nullptr};
    std::string_view application_name{"ff"};
    bool is_running{false};
    bool is_minimized{false};
    bool swapchain_resize_requested{false};

    // owned directly and managed by application
    graphics::GraphicsContext context{};
    graphics::Swapchain swapchain{};
    VkSurfaceKHR surface{VK_NULL_HANDLE};

};

} // namespace ff