#include "input_system.h"

namespace ff::input {

void
InputManager::register_window(SDL_Window *window) {
    SDL_WindowID id = SDL_GetWindowID(window);
    m_window_states[id] = InputSystemState{};
}

void
InputManager::unregister_window(SDL_Window *window) {
    SDL_WindowID id = SDL_GetWindowID(window);
    m_window_states.erase(id);
}

void
InputManager::prepare_for_new_frame() {
    for (auto &[id, state] : m_window_states) {
        state.keyboard.previous = state.keyboard.current;
        state.mouse.previous = state.mouse.current;
        state.mouse.scroll_x = 0.0f;
        state.mouse.scroll_y = 0.0f;
    }
}

// --- KEYBOARD QUERIES ---

bool
InputManager::is_key_down(SDL_WindowID window_id, KeyboardKeys::Value key) const {
    auto it = m_window_states.find(window_id);
    if (it != m_window_states.end() && key < SDL_SCANCODE_COUNT) {
        return it->second.keyboard.current[key];
    }
    return false;
}

bool
InputManager::is_key_up(SDL_WindowID window_id, KeyboardKeys::Value key) const {
    return !is_key_down(window_id, key);
}

bool
InputManager::is_key_pressed(SDL_WindowID window_id, KeyboardKeys::Value key) const {
    auto it = m_window_states.find(window_id);
    if (it != m_window_states.end() && key < SDL_SCANCODE_COUNT) {
        return it->second.keyboard.current[key] && !it->second.keyboard.previous[key];
    }
    return false;
}

bool
InputManager::is_key_released(SDL_WindowID window_id, KeyboardKeys::Value key) const {
    auto it = m_window_states.find(window_id);
    if (it != m_window_states.end() && key < SDL_SCANCODE_COUNT) {
        return !it->second.keyboard.current[key] && it->second.keyboard.previous[key];
    }
    return false;
}

// --- MOUSE BUTTON QUERIES ---

bool
InputManager::is_mouse_button_down(SDL_WindowID window_id, MouseButton::Value button) const {
    auto it = m_window_states.find(window_id);
    if (it != m_window_states.end() && button >= 0 && button < MouseButton::COUNT) {
        return it->second.mouse.current[button];
    }
    return false;
}

bool
InputManager::is_mouse_button_up(SDL_WindowID window_id, MouseButton::Value button) const {
    return !is_mouse_button_down(window_id, button);
}

bool
InputManager::is_mouse_button_pressed(SDL_WindowID window_id, MouseButton::Value button) const {
    auto it = m_window_states.find(window_id);
    if (it != m_window_states.end() && button >= 0 && button < MouseButton::COUNT) {
        return it->second.mouse.current[button] && !it->second.mouse.previous[button];
    }
    return false;
}

bool
InputManager::is_mouse_button_released(SDL_WindowID window_id, MouseButton::Value button) const {
    auto it = m_window_states.find(window_id);
    if (it != m_window_states.end() && button >= 0 && button < MouseButton::COUNT) {
        return !it->second.mouse.current[button] && it->second.mouse.previous[button];
    }
    return false;
}

// --- MOUSE POSITION & SCROLL GETTERS ---

f32
InputManager::get_mouse_x(SDL_WindowID window_id) const {
    auto it = m_window_states.find(window_id);
    return (it != m_window_states.end()) ? it->second.mouse.x : 0.0f;
}

f32
InputManager::get_mouse_y(SDL_WindowID window_id) const {
    auto it = m_window_states.find(window_id);
    return (it != m_window_states.end()) ? it->second.mouse.y : 0.0f;
}

f32
InputManager::get_mouse_scroll_x(SDL_WindowID window_id) const {
    auto it = m_window_states.find(window_id);
    return (it != m_window_states.end()) ? it->second.mouse.scroll_x : 0.0f;
}

f32
InputManager::get_mouse_scroll_y(SDL_WindowID window_id) const {
    auto it = m_window_states.find(window_id);
    return (it != m_window_states.end()) ? it->second.mouse.scroll_y : 0.0f;
}

// --- PREVIOUS FRAME KEYBOARD QUERIES ---

bool
InputManager::was_key_down(SDL_WindowID window_id, KeyboardKeys::Value key) const {
    auto it = m_window_states.find(window_id);
    if (it != m_window_states.end() && key < SDL_SCANCODE_COUNT) {
        return it->second.keyboard.previous[key];
    }
    return false;
}

bool
InputManager::was_key_up(SDL_WindowID window_id, KeyboardKeys::Value key) const {
    return !was_key_down(window_id, key);
}

// --- PREVIOUS FRAME MOUSE BUTTON QUERIES ---

bool
InputManager::was_mouse_button_down(SDL_WindowID window_id, MouseButton::Value button) const {
    auto it = m_window_states.find(window_id);
    if (it != m_window_states.end() && button >= 0 && button < MouseButton::COUNT) {
        return it->second.mouse.previous[button];
    }
    return false;
}

bool
InputManager::was_mouse_button_up(SDL_WindowID window_id, MouseButton::Value button) const {
    return !was_mouse_button_down(window_id, button);
}

void
InputManager::handle_event(const SDL_Event &event) {
    switch (event.type) {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP: {
        auto it = m_window_states.find(event.key.windowID);
        if (it != m_window_states.end()) {
            bool is_pressed = (event.type == SDL_EVENT_KEY_DOWN);
            if (event.key.scancode < SDL_SCANCODE_COUNT) {
                it->second.keyboard.current[event.key.scancode] = is_pressed;
            }
        }
        break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP: {
        auto it = m_window_states.find(event.button.windowID);
        if (it != m_window_states.end()) {
            bool is_pressed = (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN);
            i32 btn = event.button.button - 1; // SDL buttons start at 1
            if (btn >= 0 && btn < MouseButton::COUNT) {
                it->second.mouse.current[btn] = is_pressed;
            }
        }
        break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
        auto it = m_window_states.find(event.motion.windowID);
        if (it != m_window_states.end()) {
            it->second.mouse.x = event.motion.x;
            it->second.mouse.y = event.motion.y;
        }
        break;
    }
    case SDL_EVENT_MOUSE_WHEEL: {
        auto it = m_window_states.find(event.wheel.windowID);
        if (it != m_window_states.end()) {
            it->second.mouse.scroll_x = event.wheel.x;
            it->second.mouse.scroll_y = event.wheel.y;
        }
        break;
    }
    }
}

} // namespace ff::input