#define SDL_MAIN_HANDLED 1
#include "../engine/EngineAPI.h"
// #include <SDL3/SDL_vulkan.h>
#include <stdio.h>
#include <stdbool.h>

int main(int argc, char* argv[]) {
    vkb::DeviceError error{};
    // 1. Inicjalizacja wideo SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Inicjalizacja SDL nieudana: %s\n", SDL_GetError());
        return 1;
    }

    const char* name = "";
#if defined(_DEBUG)
    name = "ff window debug";
#elif defined(NDEBUG)
    name = "ff window release";
#endif
    // 2. NAJPIERW TWORZYMY OKNO - System zarezerwuje pamięć okna pod kontekst Vulkan
    SDL_Window* window = SDL_CreateWindow(
        name,
        800, 
        600, 
        SDL_WINDOW_VULKAN
    );

    if (window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Nie udalo sie utworzyc okna: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // 3. DOPIERO TERAZ ŁADUJEMY BIBLIOTEKĘ VULKAN
    if (!SDL_Vulkan_LoadLibrary(NULL)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Nie udalo sie zaladowac Vulkana: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 4. Pobranie i wypisanie rozszerzeń instancji
    Uint32 extensionCount = 0;
    const char * const *extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
    if (!extensions) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Brak rozszerzen: %s\n", SDL_GetError());
    } else {
        printf("\n=== VULKAN URUCHOMIONY POMYSLNIE ===\n");
        printf("Wymagane rozszerzenia Vulkan przez SDL (%d):\n", extensionCount);
        for (Uint32 i = 0; i < extensionCount; i++) {
            printf(" - %s\n", extensions[i]);
        }
        printf("====================================\n\n");
    }

    // 5. Pętla programu (teraz okno utrzyma się na ekranie)
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
    }

    // 6. Czyszczenie
    SDL_DestroyWindow(window);
    SDL_Vulkan_UnloadLibrary();
    SDL_Quit();

    return 0;
}