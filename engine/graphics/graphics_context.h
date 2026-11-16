#pragma once

#include "VkBootstrap.h"
#include "core/common/common_headers.h"

#include "swapchain.h"

namespace ff::graphics {
struct GraphicsContext {
    vkb::Instance            vkb_instance{};
    vkb::Device              vkb_device{};

    VkDevice                 device{ VK_NULL_HANDLE };
    VkPhysicalDevice         physical_device{ VK_NULL_HANDLE };

    VkSurfaceKHR             surface{ VK_NULL_HANDLE };
    Swapchain                swapchain;

    VkInstance               instance{ VK_NULL_HANDLE };
    VkDebugUtilsMessengerEXT debug_messenger{ VK_NULL_HANDLE };

    VkQueue                  graphics_queue{ VK_NULL_HANDLE };
    VkQueue                  present_queue{ VK_NULL_HANDLE };
    VkQueue                  compute_queue{ VK_NULL_HANDLE };
    VkQueue                  transfer_queue{ VK_NULL_HANDLE };

    u32                      present_family_index{ 0 };
    u32                      graphics_family_index{ 0 };
    u32                      compute_family_index{ 0 };
    u32                      transfer_family_index{ 0 };

    bool initialize(const char *app_name, u32 ext_count, const char *const *extensions, SDL_Window *window);
    void destroy();

    bool create_swapchain(u32 width, u32 height);
    void recreate_swapchain(u32 width, u32 height); 
    void destroy_swapchain();

  private:
    bool init_instance(const char *app_name, u32 ext_count, const char *const *extensions);
    bool init_device(VkSurfaceKHR surface);
};

} // namespace ff::graphics