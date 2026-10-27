#pragma once

/*
// step 1
uint32_t extension_count = 0;
const char* const* sdl_extensions = SDL_Vulkan_GetInstanceExtensions(&extension_count);

vkb::InstanceBuilder instance_builder;
auto instance_ret = instance_builder
    .set_app_name("SDL3 Vulkan App")
    .require_api_version(1, 3, 0) // Np. Vulkan 1.3
    .enable_extensions(extension_count, sdl_extensions) 
#if !defined(NDEBUG)
    .request_validation_layers()
#endif
    .build();

if (!instance_ret) {
    FERROR("Failed to create Vulkan instance: %s", instance_ret.error().message().c_str());
    return;
}

vkb::Instance vkb_instance = instance_ret.value();
VkInstance instance = vkb_instance.instance; // Surowy uchwyt dla SDL3

// step 2
VkSurfaceKHR surface = VK_NULL_HANDLE;

if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface)) {
    FERROR("Failed to create Vulkan surface! SDL Error: %s", SDL_GetError());
    return;
}

// step 3
vkb::PhysicalDeviceSelector selector{ vkb_instance };
auto phys_device_ret = selector
    .set_surface(surface) // <-- vkb sets surface there
    .set_minimum_version(1, 3)
    .select();

if (!phys_device_ret) {
    FERROR("Failed to select Physical Device: %s", phys_device_ret.error().message().c_str());
    return;
}
vkb::PhysicalDevice vkb_phys_device = phys_device_ret.value();

vkb::DeviceBuilder device_builder{ vkb_phys_device };
auto device_ret = device_builder.build();

if (!device_ret) {
    FERROR("Failed to create Logical Device: %s", device_ret.error().message().c_str());
    return;
}
vkb::Device vkb_device = device_ret.value();

VkPhysicalDevice physical_device = vkb_phys_device.physical_device;
VkDevice device = vkb_device.device;

*/