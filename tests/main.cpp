#define SDL_MAIN_HANDLED 1
#include "../engine/EngineAPI.h"
// #include <SDL3/SDL_vulkan.h>
#include <stdbool.h>
#include <stdio.h>

int
main(int argc, char *argv[]) {
    initialize_logging();

    vkb::DeviceError error{};
    // 1. Inicjalizacja wideo SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Inicjalizacja SDL nieudana: %s\n", SDL_GetError());
        return 1;
    }

    const char *name = "ff window default";

#if defined(NDEBUG)
    name = "ff window release";
#else
    name = "ff window debug"; // Fallback for Debug (when NDEBUG is not defined)
#endif
    // 2. NAJPIERW TWORZYMY OKNO - System zarezerwuje pamięć okna pod kontekst Vulkan
    SDL_Window *window = SDL_CreateWindow(name, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, SDL_WINDOW_VULKAN);

    if (window == NULL) {
        // SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Nie udalo sie utworzyc okna: %s\n", SDL_GetError());
        FERROR("SDL failed to create window: {}", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    FInputMgr.register_window(window);

    // 3. DOPIERO TERAZ ŁADUJEMY BIBLIOTEKĘ VULKAN
    if (!SDL_Vulkan_LoadLibrary(NULL)) {
        // SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Nie udalo sie zaladowac Vulkana: %s\n", SDL_GetError());
        FERROR("SDL failed to load vulkan : {}", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 4. Pobranie i wypisanie rozszerzeń instancji
    Uint32 extensionCount = 0;
    const char *const *extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
    if (!extensions) {
        // SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Brak rozszerzen: %s\n", SDL_GetError());
        FERROR("Failed to load extensions: {}", SDL_GetError());
    } else {
        printf("\n=== VULKAN URUCHOMIONY POMYSLNIE ===\n");
        printf("Wymagane rozszerzenia Vulkan przez SDL (%d):\n", extensionCount);
        for (Uint32 i = 0; i < extensionCount; i++) {
            printf("%s\n", extensions[i]);
        }
        printf("====================================\n\n");
    }

    FINFO("initialized SDL window successfully.");
    SDL_WindowID window_id = SDL_GetWindowID(window);

    // 5. Pętla programu (teraz okno utrzyma się na ekranie)
    bool running = true;
    SDL_Event event;


    while (running) {
        FInputMgr.prepare_for_new_frame();
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }

            // feed events into our input manager
            FInputMgr.handle_event(event);
        }

        if (FInputMgr.is_key_pressed(window_id, SDL_SCANCODE_ESCAPE)) {
            FINFO("ESC key pressed - exiting loop.");
            running = false;
        }

        if (FInputMgr.is_key_pressed(window_id, SDL_SCANCODE_SPACE)) {
            FINFO("Space bar pressed this frame!");
        }

        // Example: Checking mouse button clicks
        if (FInputMgr.is_mouse_button_pressed(window_id, 0)) { // Left click (0)
            FINFO("Left mouse clicked at coordinates: ({}, {})", 
                  FInputMgr.get_mouse_x(window_id), 
                  FInputMgr.get_mouse_y(window_id));
        }
    }

    // 6. Czyszczenie
    FInputMgr.unregister_window(SDL_GetWindowFromID(window_id)); // SDL_GetWindowFromID(window_id)
    SDL_DestroyWindow(window);
    SDL_Vulkan_UnloadLibrary();
    SDL_Quit();

    FINFO("Closed SDL window.");

    shutdown_logging();
    return 0;
}