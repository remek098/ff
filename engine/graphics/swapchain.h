#pragma once

#include "graphics_context.h"

namespace ff::graphics {
class Swapchain {
public:
    Swapchain() = default;
    void create(GraphicsContext& context, VkSurfaceKHR surface, u32 width, u32 height);
    void recreate(GraphicsContext& context, VkSurfaceKHR surface, u32 width, u32 height);
    void destroy(GraphicsContext& context);

    std::vector<VkImage> get_images() { return vkb_swapchain.get_images().value(); }
    std::vector<VkImageView> get_image_views() { return vkb_swapchain.get_image_views().value(); }
    VkExtent2D get_extent() const { return vkb_swapchain.extent; }
    VkFormat get_format() const { return vkb_swapchain.image_format; }
    VkSwapchainKHR get_handle() const { return vkb_swapchain.swapchain; }

private:
    vkb::Swapchain vkb_swapchain{};
};

}