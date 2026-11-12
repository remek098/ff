#pragma once

#define SDL_MAIN_HANDLED 1
#include "EngineAPI.h"
#include "core/components/transform_component.h"
#include "core/components/entity_component.h"

#include <ctime>

using namespace ff;

class TestGame : public ff::Application {
  public:
    TestGame() : ff::Application("Entity Test Window", 1280, 720) {}
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

        register_mouse_binding("Player.Shoot",
                               {}, // Optional modifier: Shift + Left Click
                               input::MouseButton::Value::Left,
                               [this]() {
                                   FINFO("Player clicked Left Mouse Button at ({}, {})",
                                         FInputManager.get_mouse_x(get_window_id()), // Or query coordinates globally
                                         FInputManager.get_mouse_y(get_window_id()));
                               });

        srand((u32)time(nullptr));
        return true;
    }

    void update(f32 dt) override {
        // Call base engine update if needed
        ff::Application::update(dt);

        // Put your game-specific per-frame logic here
        if (!FInputManager.was_key_down(get_window_id(), input::KeyboardKeys::Q)) {
            for (u32 i = 0; i < 10000; ++i) {
                create_random();
                remove_random();
                m_num_entities = (u32)m_entities.size();
            }
            FTRACE("Entities created {}", m_added);
            FTRACE("Entities removed {}", m_removed);
        }
    }

  private:
    void create_random() {
        u32 count = rand() % 20; // random number between 0-20
        if (m_entities.empty()) count = 1000;

        components::TransformComponentInitInfo transform_info{};
        entity::EntityInfo                     entity_info{ &transform_info };

        while (count > 0) {
            ++m_added;
            entity::Entity entity = entity::create_entity(entity_info);
            FASSERT(entity.is_valid() && id::is_valid(entity.get_id()), "Randomly created entity invalid during test.");
            m_entities.push_back(entity);
            FASSERT(entity::is_alive(entity.get_id()), "Randomly created entity is not alive.");

            --count;
        }
    }

    void remove_random() {
        u32 count = rand() % 20;
        if (m_entities.size() < 1000) return;
        while (count > 0) {
            // [0, num_entities]
            const u32      index  = (u32)rand() % (u32)m_entities.size();
            entity::Entity entity = m_entities[index];
            assert(entity.is_valid() && id::is_valid(entity.get_id()));
            if (entity.is_valid()) {
                entity::remove_entity(entity.get_id());
                m_entities.erase(m_entities.begin() + index);
                FASSERT(!entity::is_alive(entity.get_id()),
                        "Tried to remove randomly generated entity that is not alive.");
                ++m_removed;
            }

            --count;
        }
    }

  private:
    std::vector<entity::Entity> m_entities;

    u32                         m_added        = 0;
    u32                         m_removed      = 0;
    u32                         m_num_entities = 0;
};