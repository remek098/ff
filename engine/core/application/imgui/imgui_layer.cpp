#include "imgui_layer.h"
#include "graphics/graphics_context.h"
#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"

namespace ff::editor {
void
ImGuiLayer::stop_rendering() {
    {
        std::lock_guard<std::mutex> lock(frame_mutex_);
        shutting_down_ = true;
    }
    frame_cv_.notify_all();
}

void
ImGuiLayer::cleanup_payload() {
    {
        std::lock_guard<std::mutex> lock(frame_mutex_);
        payload_.draw_data = nullptr;
        payload_.ready     = false;
    }
    frame_cv_.notify_all();
}

ElementId
ImGuiLayer::register_element(ff::uptr<ImGuiEditorElement> element) {
    if (!element) return 0;
    std::lock_guard<std::mutex> lock(elements_mutex_);
    const ElementId             id = next_element_id++;
    elements_to_draw_.push_back({ id, std::move(element) });
    return id;
}

void
ImGuiLayer::unregister_element(ElementId id) {
    std::lock_guard<std::mutex> lock(elements_mutex_);
    std::erase_if(elements_to_draw_, [id](const RegisteredElement &entry) { return entry.id == id; });
}

void
ImGuiLayer::clear_elements() {
    std::lock_guard<std::mutex> lock(elements_mutex_);
    elements_to_draw_.clear();
}

void
ImGuiLayer::update() {
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // Full-screen DockSpace (UE5-style editor shell)
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    std::vector<ImGuiEditorElement *> active_elements;
    {
        std::lock_guard<std::mutex> lock(elements_mutex_);
        active_elements.reserve(elements_to_draw_.size());
        for (const auto &entry : elements_to_draw_) {
            if (entry.element) {
                active_elements.push_back(entry.element.get());
            }
        }
    }

    for (ImGuiEditorElement *elem : active_elements) {
        elem->update();
    }

    ImGui::Render();

    // Hand off ImDrawData to render_thread (guarded by frame_mutex_ / frame_cv_)
    {
        std::unique_lock<std::mutex> lock(frame_mutex_);
        frame_cv_.wait(lock, [this] { return !payload_.ready || shutting_down_; });
        if (shutting_down_) {
            return;
        }
        payload_.draw_data = ImGui::GetDrawData();
        payload_.ready     = (payload_.draw_data != nullptr);
    }
    frame_cv_.notify_one();
}

void
ImGuiLayer::render_draw_data(VkCommandBuffer cmd) {
    ImDrawData *frame_draw_data = nullptr;
    {
        std::lock_guard<std::mutex> lock(frame_mutex_);
        if (payload_.ready) {
            frame_draw_data = payload_.draw_data;
        }
    }

    if (frame_draw_data != nullptr && frame_draw_data->CmdListsCount > 0) {
        ImGui_ImplVulkan_RenderDrawData(frame_draw_data, cmd);
    }
}

void
ImGuiLayer::initialize(SDL_Window *window, graphics::GraphicsContext &gcontext) {
    {
        std::lock_guard<std::mutex> lock(frame_mutex_);
        payload_.draw_data = nullptr;
        payload_.ready     = false;
        shutting_down_     = false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // Keep ViewportsEnable off for now while using Option A with render_thread.

    ImGui_ImplSDL3_InitForVulkan(window);

    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
    };

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets       = 1000;
    pool_info.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes));
    pool_info.pPoolSizes    = pool_sizes;

    vkCreateDescriptorPool(gcontext.vkb_device.device, &pool_info, nullptr, &imgui_pool_);

    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.ApiVersion          = VK_API_VERSION_1_4; // Match your Vulkan instance/device version
    init_info.Instance            = gcontext.vkb_instance.instance;
    init_info.PhysicalDevice      = gcontext.vkb_device.physical_device;
    init_info.Device              = gcontext.vkb_device.device;
    init_info.QueueFamily         = gcontext.graphics_family_index;
    init_info.Queue               = gcontext.graphics_queue;
    init_info.DescriptorPool      = imgui_pool_;
    init_info.MinImageCount       = 2;
    init_info.ImageCount          = static_cast<uint32_t>(gcontext.swapchain.get_images().size());
    init_info.UseDynamicRendering = true;

    VkFormat color_format         = gcontext.swapchain.get_format(); // Match your swapchain image format
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo = {
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount    = 1,
        .pColorAttachmentFormats = &color_format,
    };

    ImGui_ImplVulkan_Init(&init_info);
}

void
ImGuiLayer::shutdown(graphics::GraphicsContext &gcontext) {
    elements_to_draw_.clear();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(gcontext.vkb_device.device, imgui_pool_, nullptr);
}
} // namespace ff::editor