#pragma once

#include "VkBootstrap.h"
#include "core/common/common_headers.h"

#include "swapchain.h"

namespace ff::graphics {
struct GraphicsContext {
    vkb::Instance vkb_instance{};
    vkb::Device   vkb_device{};
    VkSurfaceKHR  surface{ VK_NULL_HANDLE };

    Swapchain     swapchain;

    VkQueue       graphics_queue{ VK_NULL_HANDLE };
    VkQueue       present_queue{ VK_NULL_HANDLE };

    bool          initialize(const char *app_name, u32 ext_count, const char *const *extensions, SDL_Window *window);
    void          destroy();

  private:
    bool init_instance(const char *app_name, u32 ext_count, const char *const *extensions);
    bool init_device(VkSurfaceKHR surface);
};

} // namespace ff::graphics