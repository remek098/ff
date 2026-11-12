#define TEST_ENTITY_COMPONENTS 0
#define TEST_WINDOW 1
#define TEST_RENDERER 0

#if TEST_WINDOW
#include "window_test.h"
#elif TEST_ENTITY_COMPONENTS
#include "entity_component_test.h"
#else
#error One of the test must be enabled. Lord have mercy upon your soul.
#endif
// #include <SDL3/SDL_vulkan.h>

int
main(int argc, char *argv[]) {
    TestGame app;
    app.run();

    return 0;
}