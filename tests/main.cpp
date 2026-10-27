#define SDL_MAIN_HANDLED 1
#include "../engine/EngineAPI.h"
// #include <SDL3/SDL_vulkan.h>
#include <stdbool.h>
#include <stdio.h>

int
main(int argc, char *argv[]) {
    ff::Application app{};
    app.run();
    
    return 0;
}