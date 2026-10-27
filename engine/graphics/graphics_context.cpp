#include "graphics_context.h"

#include "core/logging/logger.h"

#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"

namespace ff::graphics {

static VKAPI_ATTR VkBool32 VKAPI_CALL
vulkan_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                      VkDebugUtilsMessageTypeFlagsEXT messageType,
                      const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
    // You can format the message prefix based on the message type (Validation, Performance, etc.)
    const char *type_str = "Vulkan";
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
        type_str = "Validation";
    } else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
        type_str = "Performance";
    }

    // Route the message payload directly into your spdlog infrastructure
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        FERROR("[{}] {}", type_str, pCallbackData->pMessage);
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        FWARN("[{}] {}", type_str, pCallbackData->pMessage);
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        FINFO("[{}] {}", type_str, pCallbackData->pMessage);
    }

    // Always return VK_FALSE. Returning VK_TRUE skips the driver action that triggered the layer message.
    return VK_FALSE;
}

bool
GraphicsContext::init_instance(const char *app_name, uint32_t ext_count, const char *const *extensions) {
    vkb::InstanceBuilder instance_builder;
    auto instance_ret = instance_builder.set_app_name(app_name)
                            .require_api_version(1, 3, 0)
                            .enable_extensions(ext_count, extensions)
#if !defined(NDEBUG)
                            .request_validation_layers()
                            .set_debug_callback(vulkan_debug_callback)
#endif
                            .build();

    if (!instance_ret) {
        FERROR("Failed to create Vulkan instance: {}", instance_ret.error().message().c_str());
        return false;
    }

    // retrieve created by vkb VkInstance so that we can retrieve VkSurfaceKHR from SDL window we pass as a pointer.
    vkb_instance = instance_ret.value();

    FINFO("vulkan instance created successfully.");
    return true;
}

bool
GraphicsContext::init_device(VkSurfaceKHR surface) {
    vkb::PhysicalDeviceSelector selector{vkb_instance};
    auto phys_device_ret = selector.set_surface(surface).set_minimum_version(1, 3).select();

    if (!phys_device_ret)
        return false;

    vkb::DeviceBuilder device_builder{phys_device_ret.value()};
    auto device_ret = device_builder.build();

    if (!device_ret)
        return false;

    vkb_device = device_ret.value();

    FINFO("vulkan device created successfully.");
    return true;
}

void
GraphicsContext::destroy() {
    vkDeviceWaitIdle(vkb_device.device);

    vkb::destroy_device(vkb_device);
    vkb::destroy_instance(vkb_instance);
    FINFO("graphics::GraphicsContext destroyed gracefully.");
}

} // namespace ff::graphics