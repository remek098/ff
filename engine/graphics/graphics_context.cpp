#include "graphics_context.h"

#include "core/logging/logger.h"

#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"

#ifdef __APPLE__
#include "vulkan/vulkan_metal.h"

#endif

namespace ff::graphics {

static VKAPI_ATTR VkBool32 VKAPI_CALL
vulkan_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                      VkDebugUtilsMessageTypeFlagsEXT             messageType,
                      const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                      void                                       *pUserData) {
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

    // Always return VK_FALSE. Returning VK_TRUE skips the driver action that triggered the layer
    // message.
    return VK_FALSE;
}

bool
GraphicsContext::init_instance(const char *app_name, uint32_t ext_count, const char *const *extensions) {
    vkb::InstanceBuilder instance_builder(vkGetInstanceProcAddr);
    auto                 instance_ret = instance_builder.set_app_name(app_name)
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

    // retrieve created by vkb VkInstance.
    vkb_instance = instance_ret.value();
    FINFO("vulkan instance created successfully.");
    return true;
}

bool
GraphicsContext::init_device(VkSurfaceKHR surface) {
    // vulkan 1.3 features
    VkPhysicalDeviceVulkan13Features features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
    features.dynamicRendering = true;
    features.synchronization2 = true;

    // vulkan 1.2 features
    VkPhysicalDeviceVulkan12Features features12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
    features12.bufferDeviceAddress = true;
    features12.descriptorIndexing  = true;

    vkb::PhysicalDeviceSelector selector{ vkb_instance };
    auto                        phys_device_ret = selector.set_surface(surface)
                                                      .set_minimum_version(1, 3)
                                                      .set_required_features_13(features)
                                                      .set_required_features_12(features12)
                                                      .select();

    if (!phys_device_ret) {
        FERROR("Couldn't find GPU that would support required features.");
        return false;
    }

    vkb::DeviceBuilder device_builder{ phys_device_ret.value() };
    auto               device_ret = device_builder.build();

    if (!device_ret) return false;

    vkb_device = device_ret.value();

    FINFO("vulkan device created successfully.");
    FINFO("[VULKAN] Present queue index: {}", vkb_device.get_queue_index(vkb::QueueType::present).value());
    FINFO("[VULKAN] Graphics queue index: {}", vkb_device.get_queue_index(vkb::QueueType::graphics).value());
    FINFO("[VULKAN] Compute queue index: {}", vkb_device.get_queue_index(vkb::QueueType::compute).value());
    FINFO("[VULKAN] Transfer queue index: {}", vkb_device.get_queue_index(vkb::QueueType::transfer).value());

    graphics_queue = vkb_device.get_queue(vkb::QueueType::graphics).value();
    present_queue  = vkb_device.get_queue(vkb::QueueType::present).value();
    return true;
}

bool
GraphicsContext::initialize(const char *app_name, u32 ext_count, const char *const *extensions, SDL_Window *window) {
    if (!init_instance(app_name, ext_count, extensions)) {
        FERROR("Failed to initialize Vulkan Instance!");
        return false;
    }

    if (!SDL_Vulkan_CreateSurface(window, vkb_instance.instance, nullptr, &surface)) {
        FERROR("Failed to create Vulkan surface! SDL Error: {}", SDL_GetError());
        return false;
    }

    if (!init_device(surface)) {
        FERROR("Failed to init graphics::GraphicsContext::vkb_device during Application init!");
        return false;
    }

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