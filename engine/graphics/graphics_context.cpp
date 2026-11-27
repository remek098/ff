#include "graphics_context.h"

#include "core/logging/logger.h"

#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

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
                                            .require_api_version(1, 4, 0)
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
    vkb_instance    = instance_ret.value();
    instance        = vkb_instance.instance;
    debug_messenger = vkb_instance.debug_messenger;
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
    features12.bufferDeviceAddress                          = true;
    features12.descriptorIndexing                           = true;
    features12.runtimeDescriptorArray                       = VK_TRUE;
    features12.descriptorBindingPartiallyBound              = VK_TRUE;
    features12.descriptorBindingVariableDescriptorCount     = VK_TRUE;
    features12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
    features12.shaderSampledImageArrayNonUniformIndexing    = VK_TRUE;

    // Shader objects
    VkPhysicalDeviceShaderObjectFeaturesEXT shader_object_features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT
    };
    shader_object_features.shaderObject = VK_TRUE;

    // Descriptor heaps
    VkPhysicalDeviceDescriptorHeapFeaturesEXT descriptor_heap_features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_FEATURES_EXT
    };
    descriptor_heap_features.descriptorHeap = VK_TRUE;

    // Mesh shading (task + mesh shaders)
    VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT
    };
    mesh_shader_features.taskShader = VK_TRUE;
    mesh_shader_features.meshShader = VK_TRUE;

    // Acceleration structures + Ray tracing pipeline
    VkPhysicalDeviceAccelerationStructureFeaturesKHR accel_features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR
    };
    accel_features.accelerationStructure = VK_TRUE;

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rt_pipeline_features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR
    };
    rt_pipeline_features.rayTracingPipeline = VK_TRUE;

    // Opacity micromaps
    VkPhysicalDeviceOpacityMicromapFeaturesEXT micromap_features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_FEATURES_EXT
    };
    micromap_features.micromap = VK_TRUE;

    // Cluster acceleration structures
    VkPhysicalDeviceClusterAccelerationStructureFeaturesNV cluster_as_features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_ACCELERATION_STRUCTURE_FEATURES_NV
    };
    cluster_as_features.clusterAccelerationStructure = VK_TRUE;

    vkb::PhysicalDeviceSelector selector{ vkb_instance };
    auto                        phys_device_ret = selector.set_surface(surface)
                                                      .set_minimum_version(1, 4)
                                                      .set_required_features_13(features)
                                                      .set_required_features_12(features12)
                                                      .add_required_extensions({
                                                          VK_EXT_SHADER_OBJECT_EXTENSION_NAME,
                                                          VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME,
                                                          VK_EXT_MESH_SHADER_EXTENSION_NAME,
                                                          VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
                                                          VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
                                                          VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
                                                          VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME,
                                                          VK_NV_CLUSTER_ACCELERATION_STRUCTURE_EXTENSION_NAME,
                                                      })
                                                      // Required extension feature structs
                                                      .add_required_extension_features(shader_object_features)
                                                      .add_required_extension_features(descriptor_heap_features)
                                                      .add_required_extension_features(mesh_shader_features)
                                                      .add_required_extension_features(accel_features)
                                                      .add_required_extension_features(rt_pipeline_features)
                                                      .add_required_extension_features(micromap_features)
                                                      .add_required_extension_features(cluster_as_features)
                                                      .select();

    if (!phys_device_ret) {
        FERROR("Couldn't find GPU that would support required features.");
        return false;
    }

    vkb::DeviceBuilder device_builder{ phys_device_ret.value() };
    auto               device_ret = device_builder.build();

    if (!device_ret) return false;

    vkb_device      = device_ret.value();
    device          = vkb_device.device;
    physical_device = vkb_device.physical_device;

    FINFO("vulkan device created successfully.");
    FINFO("[VULKAN] Present queue index: {}", vkb_device.get_queue_index(vkb::QueueType::present).value());
    FINFO("[VULKAN] Graphics queue index: {}", vkb_device.get_queue_index(vkb::QueueType::graphics).value());
    FINFO("[VULKAN] Compute queue index: {}", vkb_device.get_queue_index(vkb::QueueType::compute).value());
    FINFO("[VULKAN] Transfer queue index: {}", vkb_device.get_queue_index(vkb::QueueType::transfer).value());

    present_family_index  = vkb_device.get_queue_index(vkb::QueueType::present).value();
    graphics_family_index = vkb_device.get_queue_index(vkb::QueueType::graphics).value();
    compute_family_index  = vkb_device.get_queue_index(vkb::QueueType::compute).value();
    transfer_family_index = vkb_device.get_queue_index(vkb::QueueType::transfer).value();

    graphics_queue        = vkb_device.get_queue(vkb::QueueType::graphics).value();
    present_queue         = vkb_device.get_queue(vkb::QueueType::present).value();
    compute_queue         = vkb_device.get_queue(vkb::QueueType::compute).value();

    //
    auto transfer_res = vkb_device.get_queue(vkb::QueueType::transfer);
    if (transfer_res) {
        transfer_queue        = transfer_res.value();
        transfer_family_index = vkb_device.get_queue_index(vkb::QueueType::transfer).value();
    } else {
        transfer_queue        = graphics_queue;
        transfer_family_index = graphics_family_index;
    }
    return true;
}

bool
GraphicsContext::create_swapchain(u32 width, u32 height) {
    swapchain.create(*this, width, height);

    if (swapchain.get_handle() == VK_NULL_HANDLE) return false;
    return true;
}

void
GraphicsContext::destroy_swapchain() {
    if (swapchain.get_handle() != VK_NULL_HANDLE) swapchain.destroy(*this);
}

void
GraphicsContext::recreate_swapchain(u32 width, u32 height) {
    swapchain.recreate(*this, width, height);
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

    swapchain.destroy(*this);
    if (surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(instance, surface, nullptr);
    }

    vkb::destroy_device(vkb_device);
    device          = VK_NULL_HANDLE;
    physical_device = VK_NULL_HANDLE;
    // vkb::destroy_debug_utils_messenger(instance, debug_messenger);
    surface         = VK_NULL_HANDLE;
    debug_messenger = VK_NULL_HANDLE;
    graphics_queue  = VK_NULL_HANDLE;
    present_queue   = VK_NULL_HANDLE;
    vkb::destroy_instance(vkb_instance);
    FINFO("graphics::GraphicsContext destroyed gracefully.");
}

} // namespace ff::graphics