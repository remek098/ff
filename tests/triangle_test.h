#pragma once

#define SDL_MAIN_HANDLED 1
#include "../engine/EngineAPI.h"
#include "components/entity_component.h"
#include "components/transform_component.h"
#include "components/script_component.h"

#include "graphics/helpers/vk_initializers.h"

using namespace ff;
using namespace ff::entity;


class TestGame : public ff::Application {
  public:
    TestGame() : ff::Application("Test Game Window", 1280, 720) {}
    ~TestGame() override = default;

  protected:
    bool init(const char *name, u32 width, u32 height) override {
        // 1. Always call the base engine init first so window & Vulkan spin up
        if (!ff::Application::init(name, width, height)) {
            return false;
        }

        register_mouse_binding("Player.Shoot", {}, input::MouseButton::Value::Left, [this]() {
            FINFO("Player clicked Left Mouse Button at ({}, {})",
                  FInputManager.get_mouse_x(get_window_id()), // Or query coordinates globally
                  FInputManager.get_mouse_y(get_window_id()));
        });

        return true;
    }

    void shutdown() override {
        Application::shutdown();
    }

    void update(f32 dt) override {
        // Call base engine update if needed
        ff::Application::update(dt);

        // Put your game-specific per-frame logic here
    }

  private:
};