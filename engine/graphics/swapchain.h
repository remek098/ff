#pragma once

#include "VkBootstrap.h"
#include "common/primitive_types.h"
// #include "graphics_context.h"

namespace ff::graphics {
struct GraphicsContext;
class Swapchain {
  public:
    Swapchain() = default;
    void                      create(GraphicsContext &context, u32 width, u32 height);
    void                      recreate(GraphicsContext &context, u32 width, u32 height);
    void                      destroy(GraphicsContext &context);

    std::vector<VkImage>     &get_images() { return images; }
    std::vector<VkImageView> &get_image_views() { return image_views; }
    VkExtent2D                get_extent() const { return extent; }
    VkFormat                  get_format() const { return image_format; }
    VkSwapchainKHR            get_handle() const { return swapchain; }

  public:
    VkFormat   image_format{ VK_FORMAT_B8G8R8A8_UNORM };
    VkExtent2D extent;

  private:
    vkb::Swapchain           vkb_swapchain{};

    VkSwapchainKHR           swapchain{ VK_NULL_HANDLE };

    std::vector<VkImage>     images;
    std::vector<VkImageView> image_views;
};

} // namespace ff::graphics