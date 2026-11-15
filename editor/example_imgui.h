#pragma once

/*
Step 1: Initialize ImGui and Enable Docking.


#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"

// 1. Create ImGui context
IMGUI_CHECKVERSION();
ImGui::CreateContext();
ImGuiIO& io = ImGui::GetIO(); 
io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking branch features
io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;    // Optional: allows floating windows outside main OS window

// 2. Setup SDL3 platform backend
ImGui_ImplSDL3_InitForVulkan(window); // window is your SDL_Window* pointer

// 3. Create a dedicated Descriptor Pool for ImGui 
// (ImGui requires descriptors for fonts and textures)
VkDescriptorPoolSize pool_sizes[] = {
    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 }
};
VkDescriptorPoolCreateInfo pool_info = {};
pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
pool_info.maxSets = 1000;
pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
pool_info.pPoolSizes = pool_sizes;

VkDescriptorPool imgui_pool;
vkCreateDescriptorPool(device, &pool_info, nullptr, &imgui_pool);

// 4. Setup Vulkan renderer backend
ImGui_ImplVulkan_InitInfo init_info = {};
init_info.Instance = gcontext.vkb_instance.instance;
init_info.PhysicalDevice = gcontext.vkb_device.physical_device;
init_info.Device = gcontext.vkb_device.device;
init_info.Queue = gcontext.graphics_queue; // Make sure you store your VkQueue handle!
init_info.DescriptorPool = imgui_pool;
init_info.MinImageCount = 2;
init_info.ImageCount = swapchain_image_count;
init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

// REQUIRED because you enabled dynamic rendering:
init_info.UseDynamicRendering = true;
init_info.PipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &swapchain_image_format; // e.g., VK_FORMAT_B8G8R8A8_SRGB

ImGui_ImplVulkan_Init(&init_info);

// 5. Upload fonts (requires a temporary command buffer submission)
// ImGui_ImplVulkan_CreateFontsTexture();


*/

/*
PART 2: Handle SDL3 events.

SDL_Event event;
bool done = false;
while (!done) {
    while (SDL_PollEvent(&event)) {
        // Pass events to ImGui for mouse, keyboard, and window focus handling
        ImGui_ImplSDL3_ProcessEvent(&event);
        
        if (event.type == SDL_EVENT_QUIT)
            done = true;
    }
    
    // ... update logic ...
}
*/

/*
Part 3: Record and render frames.

// 1. Start the frame
ImGui_ImplVulkan_NewFrame();
ImGui_ImplSDL3_NewFrame();
ImGui::NewFrame();

// 2. Build your DockSpace UI layout
ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
ImGui::ShowDemoWindow(); // Example window

// 3. Render ImGui internal command structures
ImGui::Render();
ImDrawData* draw_data = ImGui::GetDrawData();

// 4. Record into your Vulkan command buffer (inside your render pass / dynamic rendering block)
// vkCmdBeginRenderPass(...) or vkCmdBeginRendering(...)
ImGui_ImplVulkan_RenderDrawData(draw_data, command_buffer);
// vkCmdEndRenderPass(...) or vkCmdEndRendering(...)

// 5. Handle multi-viewports if enabled (renders floating editor windows)
if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
}
*/


/*
Part 4: Cleanup:

vkDeviceWaitIdle(device);
ImGui_ImplVulkan_Shutdown();
ImGui_ImplSDL3_Shutdown();
ImGui::DestroyContext();
vkDestroyDescriptorPool(device, imgui_pool, nullptr);
*/