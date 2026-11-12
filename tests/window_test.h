#pragma once

#define SDL_MAIN_HANDLED 1
#include "../engine/EngineAPI.h"
#include "components/entity_component.h"
#include "components/transform_component.h"
#include "components/script_component.h"

using namespace ff;
using namespace ff::entity;

#include <Windows.h>
class beep_script;
REGISTER_SCRIPT(beep_script);
class beep_script : public script::ScriptEntity {
public:
    constexpr explicit beep_script(Entity entity)
        : script::ScriptEntity(entity) {}

    virtual void begin_play() {}
    virtual void update(f32 dt) {
        static u32 i = 300;
        if ( (i++ % 100) == 0 ) Beep(i, 100);
    }
private:
};

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

        // 2. Register game-specific actions using our new dynamic binding system!
        register_key_binding("Player.Jump", {}, input::KeyboardKeys::Value::SPACE, [this]() {
            FINFO("Jump action triggered from game code!");
        });

        register_key_binding("Player.NukeScript", {}, input::KeyboardKeys::Value::N, [this]() {
            FINFO("Pressed key to nuke entity!");
            if(id::is_valid(_id) && entity::is_alive(_id)) remove_entity(_id);
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

        components::TransformComponentInitInfo transform_info{};
        components::ScriptComponentInitInfo script_info;
        script_info.script_creator = script::get_script_creator(script::StringHash()("beep_script"));
        EntityInfo info{};
        info.transform = &transform_info;
        info.script = &script_info;
        _id =  create_entity(info).get_id();
        
        return true;
    }

    void shutdown() override {
        if(id::is_valid(_id) && entity::is_alive(_id)) remove_entity(_id);
        Application::shutdown();
    }

    void update(f32 dt) override {
        // Call base engine update if needed
        ff::Application::update(dt);

        // Put your game-specific per-frame logic here
    }

    private:
        entity::EntityId _id;
};