#define SDL_MAIN_HANDLED 1
#include "EngineAPI.h"

using namespace ff;

class Editor : public ff::Application {
  public:
    Editor() : ff::Application("FF editor Window", 1280, 720) {}
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

        register_mouse_binding(
            "Player.Shoot",
            {}, // Optional modifier: Shift + Left Click
            input::MouseButton::Value::Left,
            [this]() {
                FINFO("Player clicked Left Mouse Button at ({}, {})",
                      FInputManager.get_mouse_x(get_window_id()), // Or query coordinates globally
                      FInputManager.get_mouse_y(get_window_id()));
            });

        return true;
    }

    void update(f32 dt) override {
        // Call base engine update if needed
        ff::Application::update(dt);

        // Put your game-specific per-frame logic here
    }
};

int
main(int argc, char *argv[]) {
    Editor app;
    app.run();

    return 0;
}