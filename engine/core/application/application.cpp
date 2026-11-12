#include "application.h"
#include <format>

namespace ff {

namespace {
void
set_sdl_window_title_frame_metrics(SDL_Window *window, const std::string &name, f32 dt, f32 fps) {
    const std::string title = std::format("{} | {:.2f} ms | {:.1f} FPS", name, dt * 1000.f, fps);

    SDL_SetWindowTitle(window, title.c_str());
}
} // namespace

Application::Application(const char *name, u32 width, u32 height)
    : application_name(name ? name : "ff"), window_width(width), window_height(height) { /*init(name, width, height);*/
}

namespace {
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
std::vector<GameKeyBinding>         game_key_bindings;
std::vector<GameMouseButtonBinding> game_mouse_bindings;
} // namespace

void
Application::register_key_binding(std::string                             action_name,
                                  std::vector<input::KeyboardKeys::Value> modifiers,
                                  input::KeyboardKeys::Value              key,
                                  std::function<void()>                   callback) {
    FINFO("Registering game key binding: '{}'", action_name);
    game_key_bindings.push_back({ std::move(action_name), std::move(modifiers), key, std::move(callback) });
}

void
Application::register_mouse_binding(std::string                             action_name,
                                    std::vector<input::KeyboardKeys::Value> modifiers,
                                    input::MouseButton::Value               button,
                                    std::function<void()>                   callback) {
    FINFO("Registering game mouse binding: '{}'", action_name);
    game_mouse_bindings.push_back({ std::move(action_name), std::move(modifiers), button, std::move(callback) });
}

bool
Application::init(const char *name, u32 width, u32 height) {
    initialize_logging();

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        FERROR("Couldn't even initialize SDL.");
        return false;
    }

    window =
        SDL_CreateWindow(name, width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE /*| SDL_WINDOW_MOUSE_CAPTURE*/);
    if (!window) {
        FERROR("SDL failed to create window: {}", SDL_GetError());
        SDL_Quit();
        return false;
    }

    if (!SDL_Vulkan_LoadLibrary(NULL)) {
        FERROR("SDL failed to load vulkan : {}", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    Uint32             extensionCount = 0;
    const char *const *extensions     = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
    if (!extensions) {
        FERROR("Failed to load extensions: {}", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }
    // else {
    //     printf("\n=== SDL VULKAN INITIALIZED SUCCESSFULLY ===\n");
    //     printf("Vulkan extensions required by SDL (%d):\n", extensionCount);
    //     for (Uint32 i = 0; i < extensionCount; i++) {
    //         printf("%s\n", extensions[i]);
    //     }
    //     printf("====================================\n\n");
    // }
    if (!context.init_instance(name, extensionCount, extensions)) {
        FERROR("Failed to initialize Vulkan Instance!");
        return false;
    }

    if (!SDL_Vulkan_CreateSurface(window, context.vkb_instance.instance, nullptr, &surface)) {
        FERROR("Failed to create Vulkan surface! SDL Error: {}", SDL_GetError());
        return false;
    }

    if (!context.init_device(surface)) {
        FERROR("Failed to init graphics::GraphicsContext::vkb_device during Application init!");
        return false;
    }

    VkExtent2D extent = get_window_extent();
    swapchain.create(context, surface, extent.width, extent.height);

    FInputManager.register_window(window);
    // SDL_ShowWindow(window);
    // SDL_RaiseWindow(window);

    FINFO("initialized SDL window successfully.");
    is_running = true;
    FINFO("Application initialized successfully.");

    register_default_input_bindings();

    return true;
}

void
Application::register_default_input_bindings() {
    game_mouse_bindings.clear();
    game_key_bindings.clear();
    key_bindings.clear(); // just in case if we restart application.
    key_bindings.push_back(KeyBinding(Action::Quit,
                                      {},                                 // No modifiers
                                      input::KeyboardKeys::Value::ESCAPE, // <-- Use Value namespace
                                      [this]() { on_quit_triggered(); }));

    key_bindings.push_back(
        KeyBinding(Action::SaveSpecial,
                   { input::KeyboardKeys::Value::LCTRL, input::KeyboardKeys::Value::LSHIFT }, // <-- Use Value namespace
                   input::KeyboardKeys::Value::S,                                             // <-- Use Value namespace
                   []() { FINFO("Ctrl+Shift+S pressed."); }));

    key_bindings.push_back(KeyBinding(Action::Reopen, {}, input::KeyboardKeys::Value::F11, [this]() {
        shutdown();
        init(application_name.c_str(), window_width, window_height);
        FINFO("Relaunching application...");
    }));
}

void
Application::on_quit_triggered() {
    FINFO("ESC key pressed - closing application...");
    is_running = false;
}

void
Application::handle_input() {
    triggered_actions_this_frame.clear();
    SDL_WindowID window_id = SDL_GetWindowID(window);

    // Store callbacks to run AFTER iteration finishes to prevent iterator invalidation
    std::vector<std::function<void()>> callbacks_to_run;

    // 1. handle key_bindings
    for (const auto &binding : key_bindings) {
        bool modifiers_met = true;
        for (auto mod : binding.required_down) {
            if (!FInputManager.is_key_down(window_id, mod)) {
                modifiers_met = false;
                break;
            }
        }

        if (modifiers_met && FInputManager.is_key_pressed(window_id, binding.pressed_this_frame)) {
            triggered_actions_this_frame.push_back(binding.action);
            if (binding.on_triggered) {
                callbacks_to_run.push_back(binding.on_triggered);
            }
        }
    }

    // 2. handle game-specific keybindings
    for (const auto &binding : game_key_bindings) {
        bool modifiers_met = true;
        for (auto mod : binding.required_down) {
            if (!FInputManager.is_key_down(window_id, mod)) {
                modifiers_met = false;
                break;
            }
        }

        if (modifiers_met && FInputManager.is_key_pressed(window_id, binding.pressed_this_frame)) {
            if (binding.on_triggered) {
                callbacks_to_run.push_back(binding.on_triggered);
            }
        }
    }

    // handle mouse button bindings.
    for (const auto &binding : game_mouse_bindings) {
        bool modifiers_met = true;
        for (auto mod : binding.required_down) {
            if (!FInputManager.is_key_down(window_id, mod)) {
                modifiers_met = false;
                break;
            }
        }

        if (modifiers_met && FInputManager.is_mouse_button_pressed(window_id, binding.button)) {
            if (binding.on_triggered) {
                callbacks_to_run.push_back(binding.on_triggered);
            }
        }
    }

    // Execute all collected callbacks safely *after* the loops have safely exited
    for (const auto &callback : callbacks_to_run) {
        callback();
    }

    // Example: Checking mouse button clicks
    // if (FInputManager.is_mouse_button_pressed(window_id, input::MouseButton::Left)) {
    //     FINFO("Left mouse clicked at coordinates: ({}, {})",
    //           FInputManager.get_mouse_x(window_id),
    //           FInputManager.get_mouse_y(window_id));
    // }

    // if (FInputManager.is_mouse_button_pressed(window_id, input::MouseButton::Right)) {
    //     FINFO("Right mouse clicked at coordinates: ({}, {})",
    //           FInputManager.get_mouse_x(window_id),
    //           FInputManager.get_mouse_y(window_id));
    // }
}

void
Application::handle_resize() {
    FDEBUG("Hello from handle_resize()");
    VkExtent2D extent = get_window_extent();

    // Pause rendering if minimized
    if (extent.width == 0 || extent.height == 0) {
        is_minimized = true;
        return;
    }
    is_minimized  = false;

    window_width  = extent.width;
    window_height = extent.height;

    // Recreate the swapchain safely
    vkDeviceWaitIdle(context.vkb_device.device);
    swapchain.recreate(context, surface, extent.width, extent.height);
    swapchain_resize_requested = false;

    FINFO("Resize handled.");
}

VkExtent2D
Application::get_window_extent() const {
    i32 w = 0, h = 0;
    SDL_GetWindowSizeInPixels(window, &w, &h);
    return VkExtent2D{ static_cast<u32>(w), static_cast<u32>(h) };
}

void
Application::process_sdl_events(const SDL_Event &event) {

    switch (event.type) {
    case SDL_EVENT_QUIT: {
        is_running = false;
    } break;

    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
        swapchain_resize_requested = true;
        FINFO("Swapchain resize requested.");
    } break;

    case SDL_EVENT_WINDOW_MINIMIZED: {
        is_minimized = true;
        FINFO("Window minimized");
    } break;

    case SDL_EVENT_WINDOW_RESTORED: {
        is_minimized               = false;
        swapchain_resize_requested = true;
        FINFO("Window restored");
    } break;

    default:
        break;
    }

    // feed events into our input manager
    FInputManager.handle_event(event);
}

void
Application::update(f32 dt) {
    FInputManager.prepare_for_new_frame();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        process_sdl_events(event);
    }
    handle_input();
}

void
Application::render() {
    if (swapchain_resize_requested && !is_minimized) {
        i32 width, height;
        SDL_GetWindowSizeInPixels(window, &width, &height);
        if (width > 0 && height > 0) {
            FDEBUG("TODO: recreate or request renderer to recreate a swapchain. ({}, {})", width, height);
            // swapchain_resize_requested = false;
            handle_resize();
        }
    }

    if (is_minimized) return;
}

void
Application::shutdown() {
    vkDeviceWaitIdle(context.vkb_device.device);

    // Clean up in reverse creation order
    swapchain.destroy(context);
    if (surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(context.vkb_instance.instance, surface, nullptr);
        surface = VK_NULL_HANDLE;
    }
    context.destroy();

    FInputManager.unregister_window(window); // SDL_GetWindowFromID(window_id)
    SDL_DestroyWindow(window);
    SDL_Vulkan_UnloadLibrary();
    SDL_Quit();

    FINFO("Closed SDL window.");

    shutdown_logging();
}

void
Application::run() {
    if (!init(application_name.c_str(), window_width, window_height)) {
        FERROR("Application initialization failed. Exiting.");
        return;
    }
    while (is_running) {
        timer.begin();
        // std::this_thread::sleep_for(std::chrono::milliseconds(10));
        update(timer.dt());
        render();

        if (timer.end()) set_sdl_window_title_frame_metrics(window, application_name, timer.dt_avg(), timer.fps_avg());
    }
    shutdown();
}

} // namespace ff