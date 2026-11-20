#include "swapchain.h"

#include "core/logging/logger.h"
#include "graphics_context.h"

namespace ff::graphics {
void
Swapchain::create(GraphicsContext &context, u32 width, u32 height) {
    FDEBUG("Hello from swapchain create()");
    vkb::SwapchainBuilder swapchainBuilder{ context.physical_device, context.device, context.surface };
    
    // Pass the existing swapchain handle if it exists
    if (vkb_swapchain.swapchain != VK_NULL_HANDLE) {
        swapchainBuilder.set_old_swapchain(vkb_swapchain);
    }

    auto swap_ret = swapchainBuilder
                        //.use_default_format_selection()
                        .set_desired_format(VkSurfaceFormatKHR{ .format     = image_format,
                                                                .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
                        // use vsync present mode
                        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
                        .set_desired_min_image_count(2)
                        .set_desired_extent(width, height)
                        .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                        .build();

    if (!swap_ret) {
        FERROR("Failed to create swapchain: {}", swap_ret.error().message());
        return;
    }
    if (vkb_swapchain.swapchain != VK_NULL_HANDLE) vkb::destroy_swapchain(vkb_swapchain);

    // store all necessarry stuff
    vkb_swapchain       = swap_ret.value();
    swapchain           = vkb_swapchain.swapchain;
    extent              = vkb_swapchain.extent;
    auto imgs_and_views = vkb_swapchain.get_images_and_image_views().value();
    images              = std::move(imgs_and_views.first);
    image_views         = std::move(imgs_and_views.second);

    FDEBUG("Swapchain created successfully.");
}

void
Swapchain::recreate(GraphicsContext &context, u32 width, u32 height) {
    FDEBUG("Hello from swapchain recreate()");
    if (!width || !height) return; // don't recreate if any of these is 0

    vkDeviceWaitIdle(context.device);
    vkb_swapchain.destroy_image_views(image_views);
    image_views.clear();
    images.clear();

    create(context, width, height);
    FDEBUG("Swapchain recreated: {}x{}", width, height);
}

void
Swapchain::destroy(GraphicsContext &context) {
    FDEBUG("Hello from swapchain destroy()");
    vkb_swapchain.destroy_image_views(image_views);
    image_views.clear();
    images.clear();

    if (vkb_swapchain.swapchain != VK_NULL_HANDLE) {
        vkb::destroy_swapchain(vkb_swapchain);
        vkb_swapchain.swapchain = VK_NULL_HANDLE;
        swapchain               = VK_NULL_HANDLE;
    }
    FDEBUG("Swapchain destroyed successfully.");
}

} // namespace ff::graphics
