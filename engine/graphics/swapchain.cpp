#include "swapchain.h"
#include "core/logging/logger.h"

namespace ff::graphics {
void
Swapchain::create(GraphicsContext &context, VkSurfaceKHR surface, u32 width, u32 height) {
    FDEBUG("Hello from swapchain create()");
}

void
Swapchain::recreate(GraphicsContext &context, VkSurfaceKHR surface, u32 width, u32 height) {
    FDEBUG("Hello from swapchain recreate()");
}

void
Swapchain::destroy(GraphicsContext &context) {
    FDEBUG("Hello from swapchain destroy()");
}

} // namespace ff::graphics
