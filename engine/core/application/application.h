#pragma once

#include "defines.h"

struct SDL_Window;

namespace ff {

class Application {
  public:
    DISABLE_COPY_AND_MOVE(Application);
    Application() = default;
    virtual ~Application() = default;

    void run();

  protected:
    virtual bool init();
    virtual void update(f32 dt);
    virtual void render();
    virtual void shutdown();

    private:
    virtual void handle_input();

  protected:
    bool is_running{false};
    bool is_minimized{false};
    bool swapchain_resize_requested{false};
    SDL_Window *window{nullptr};
};

} // namespace ff