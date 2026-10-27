#pragma once

#include "VkBootstrap.h"
#include "defines.h"



namespace ff::graphics {
struct GraphicsContext {
    vkb::Instance vkb_instance{};
    vkb::Device vkb_device{};

    bool init_instance(const char *app_name, u32 ext_count, const char *const *extensions);
    bool init_device(VkSurfaceKHR surface);
    void destroy();
};

} // namespace ff::graphics