#include "application.h"
#include "core/input/input_system.h"
#include "core/logging/logger.h"

#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"

namespace ff {

bool
Application::init() {
    initialize_logging();

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        FERROR("Couldn't even initialize SDL.");
        return false;
    }

    const char *name = "ff window default";

#if defined(NDEBUG)
    name = "ff window release";
#else
    name = "ff window debug"; // Fallback for Debug (when NDEBUG is not defined)
#endif

    window = SDL_CreateWindow(name, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT,
                              SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MOUSE_CAPTURE);
    if (window == nullptr) {
        FERROR("SDL failed to create window: {}", SDL_GetError());
        SDL_Quit();
        return false;
    }

    FInputManager.register_window(window);

    if (!SDL_Vulkan_LoadLibrary(NULL)) {
        FERROR("SDL failed to load vulkan : {}", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    Uint32 extensionCount = 0;
    const char *const *extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
    if (!extensions) {
        FERROR("Failed to load extensions: {}", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    } else {
        printf("\n=== SDL VULKAN INITIALIZED SUCCESSFULLY ===\n");
        printf("Vulkan extensions required by SDL (%d):\n", extensionCount);
        for (Uint32 i = 0; i < extensionCount; i++) {
            printf("%s\n", extensions[i]);
        }
        printf("====================================\n\n");
    }

    FINFO("initialized SDL window successfully.");

    is_running = true;
    FINFO("Application initialized successfully.");
    return true;
}

void
Application::handle_input() {
    SDL_WindowID window_id = SDL_GetWindowID(window);

    // NOTE: all FInputManager at this point does here is just check in arrays if the states match to certain expected
    // situation.
    //       so overall quite cheap operations to make.
    //       Obviously we want as little if statements as possible so we would only check for specific keys/combinations
    //       of keys application needs -> e.g. Esc, Ctrl+Shift+S, etc.
    if (FInputManager.is_key_pressed(window_id, SDL_SCANCODE_ESCAPE)) {
        FINFO("ESC key pressed - exiting loop.");
        is_running = false;
    }

    if (FInputManager.is_key_pressed(window_id, SDL_SCANCODE_SPACE)) {
        FINFO("Space bar pressed this frame!");
    }

    if (FInputManager.is_key_down(window_id, SDL_SCANCODE_LCTRL) &&
        FInputManager.is_key_down(window_id, SDL_SCANCODE_LSHIFT) &&
        FInputManager.is_key_pressed(window_id, SDL_SCANCODE_S)) {
        FINFO("Ctrl+Shift+S pressed.");
    }

    // Example: Checking mouse button clicks
    if (FInputManager.is_mouse_button_pressed(window_id, input::MouseButton::Left)) {
        FINFO("Left mouse clicked at coordinates: ({}, {})", FInputManager.get_mouse_x(window_id),
              FInputManager.get_mouse_y(window_id));
    }

    if (FInputManager.is_mouse_button_pressed(window_id, input::MouseButton::Right)) {
        FINFO("Right mouse clicked at coordinates: ({}, {})", FInputManager.get_mouse_x(window_id),
              FInputManager.get_mouse_y(window_id));
    }
}

void
Application::update(f32 dt) {
    SDL_Event event;

    FInputManager.prepare_for_new_frame();
    while (SDL_PollEvent(&event)) {
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
            is_minimized = false;
            swapchain_resize_requested = true;
            FINFO("Window restored");
        } break;

        default:
            break;
        }

        // feed events into our input manager
        FInputManager.handle_event(event);
    }
    handle_input();
}

void
Application::render() {
    if(swapchain_resize_requested && !is_minimized) {
        i32 width, height;
        SDL_GetWindowSizeInPixels(window, &width, &height);
        if(width > 0 && height > 0) {
            FDEBUG("TODO: recreate or request renderer to recreate a swapchain. ({}, {})", width, height);
            swapchain_resize_requested = false;
        }
    }

    if(is_minimized) return;
}

void
Application::shutdown() {
    FInputManager.unregister_window(window); // SDL_GetWindowFromID(window_id)
    SDL_DestroyWindow(window);
    SDL_Vulkan_UnloadLibrary();
    SDL_Quit();

    FINFO("Closed SDL window.");

    shutdown_logging();
}

void
Application::run() {
    init();
    while(is_running) {
        update(10.f);
        render();
    }
    shutdown();
}

} // namespace ff