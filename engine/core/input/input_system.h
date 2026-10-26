#pragma once

#include "defines.h"
#include "SDL3/SDL_scancode.h"
#include "SDL3/SDL.h"
#include <array>
#include <unordered_map>


namespace ff::input {

struct KeyboardState {
    // SDL_SCANCODE_COUNT gives the exact size needed for all standard keyboard keys
    std::array<bool, SDL_SCANCODE_COUNT> current{};
    std::array<bool, SDL_SCANCODE_COUNT> previous{};
};

struct MouseState {
    static constexpr i32 MAX_BUTTONS = 8;

    std::array<bool, MAX_BUTTONS> current{};
    std::array<bool, MAX_BUTTONS> previous{};

    // Optional: It's usually handy to store mouse positions/scroll here too
    f32 x{0.f};
    f32 y{0.f};
    f32 scroll_x{0.0f};
    f32 scroll_y{0.0f};
};

struct InputSystemState {
    KeyboardState keyboard;
    MouseState mouse;
};


class InputManager {
public:
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;
    InputManager(InputManager&&) = delete;
    InputManager& operator=(InputManager&&) = delete;

    // NOTE:  Meyers Singleton accessor -> guarantees that the instance is inline-defined -> meaning it's guaranteed to have only 1 instance.
    static InputManager& get() {
        static InputManager instance;
        return instance;
    }

    // Window Lifecycle & Events
    void register_window(SDL_Window* window);
    void unregister_window(SDL_Window* window);
    void prepare_for_new_frame();
    void handle_event(const SDL_Event& event);

    // Keyboard Queries
    bool is_key_down(SDL_WindowID window_id, SDL_Scancode key) const;
    bool is_key_up(SDL_WindowID window_id, SDL_Scancode key) const;
    bool is_key_pressed(SDL_WindowID window_id, SDL_Scancode key) const;
    bool is_key_released(SDL_WindowID window_id, SDL_Scancode key) const;

    // Previous Frame Keyboard Queries (State during last frame)
    bool was_key_down(SDL_WindowID window_id, SDL_Scancode key) const;
    bool was_key_up(SDL_WindowID window_id, SDL_Scancode key) const;


    // Mouse Button Queries
    bool is_mouse_button_down(SDL_WindowID window_id, i32 button) const;
    bool is_mouse_button_up(SDL_WindowID window_id, i32 button) const;
    bool is_mouse_button_pressed(SDL_WindowID window_id, i32 button) const;
    bool is_mouse_button_released(SDL_WindowID window_id, i32 button) const;

    // Previous Frame Mouse Button Queries (State during last frame)
    bool was_mouse_button_down(SDL_WindowID window_id, i32 button) const;
    bool was_mouse_button_up(SDL_WindowID window_id, i32 button) const;

    // Mouse Coordinates & Scroll Getters
    f32 get_mouse_x(SDL_WindowID window_id) const;
    f32 get_mouse_y(SDL_WindowID window_id) const;
    f32 get_mouse_scroll_x(SDL_WindowID window_id) const;
    f32 get_mouse_scroll_y(SDL_WindowID window_id) const;

private:
    InputManager() = default;
    ~InputManager() = default;

    std::unordered_map<SDL_WindowID, InputSystemState> m_window_states;
};



} // namespace ff::input

// Convenient macro shortcut
#define FInputManager ff::input::InputManager::get()
