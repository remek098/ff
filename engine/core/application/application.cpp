#include "application.h"
#include "components/script_component.h"
#include <format>
#include <algorithm>

// #if BUILD_WITH_EDITOR
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
// #endif

namespace ff {

namespace {
void
set_sdl_window_title_frame_metrics(SDL_Window        *window,
                                   const std::string &name,
                                   f32                game_fps,
                                   f32                render_dt,
                                   f32                render_fps) {
    const std::string title = std::format(
        "{} | Render: {:.2f} ms ({:.1f} FPS) | Game: {:.0f} Hz", name, render_dt * 1000.f, render_fps, game_fps);

    SDL_SetWindowTitle(window, title.c_str());
}
} // namespace

Application::Application(const char *name, u32 width, u32 height)
    : application_name(name ? name : "ff"), window_width(width), window_height(height) { /*init(name, width, height);*/
}

namespace {
struct EditorFramePayload {
    ImDrawData *draw_data = nullptr; // Valid until main thread calls NewFrame() again
    bool        ready     = false;
};

std::mutex                          bindings_mutex;
std::vector<GameKeyBinding>         game_key_bindings;
std::vector<GameMouseButtonBinding> game_mouse_button_bindings;

std::mutex                          imgui_frame_mutex;
std::condition_variable             imgui_frame_cv;
EditorFramePayload                  imgui_payload;
VkDescriptorPool                    imgui_pool;
bool                                imgui_shutting_down = false;
} // namespace

GameKeyBinding &
get_game_key_binding(const std::string &name) {
    std::lock_guard<std::mutex> lock(bindings_mutex);
    auto it = std::find_if(game_key_bindings.begin(), game_key_bindings.end(), [&name](const GameKeyBinding &binding) {
        return binding.name == name; // Adjust '.name' to match your struct member
    });

    if (it == game_key_bindings.end()) {
        FERROR("GameKeyBinding not found: {}", name);
        throw std::out_of_range("GameKeyBinding not found: " + name);
    }

    return *it;
}

GameMouseButtonBinding &
get_game_mouse_button_binding(const std::string &name) {
    std::lock_guard<std::mutex> lock(bindings_mutex);
    auto                        it = std::find_if(game_mouse_button_bindings.begin(),
                                                  game_mouse_button_bindings.end(),
                                                  [&name](const GameMouseButtonBinding &binding) {
                               return binding.name == name; // Adjust '.name' to match your struct member
                                                  });

    if (it == game_mouse_button_bindings.end()) {
        FERROR("GameMouseButtonBinding not found: {}", name);
        throw std::out_of_range("GameKeyBinding not found: " + name);
    }

    return *it;
}

void
Application::register_key_binding(std::string                             action_name,
                                  std::vector<input::KeyboardKeys::Value> modifiers,
                                  input::KeyboardKeys::Value              key,
                                  std::function<void()>                   callback) {
    FINFO("Registering game key binding: '{}'", action_name);
    std::lock_guard<std::mutex> lock(bindings_mutex);
    game_key_bindings.push_back({ std::move(action_name), std::move(modifiers), key, std::move(callback) });
}

void
Application::register_mouse_binding(std::string                             action_name,
                                    std::vector<input::KeyboardKeys::Value> modifiers,
                                    input::MouseButton::Value               button,
                                    std::function<void()>                   callback) {
    FINFO("Registering game mouse binding: '{}'", action_name);
    std::lock_guard<std::mutex> lock(bindings_mutex);
    game_mouse_button_bindings.push_back({ std::move(action_name), std::move(modifiers), button, std::move(callback) });
}

bool
Application::init(const char *name, u32 width, u32 height) {
    initialize_logging();

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        FERROR("Couldn't even initialize SDL.");
        return false;
    }

    window =
        SDL_CreateWindow(name, width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE /*| SDL_WINDOW_MOUSE_CAPTURE*/);
    if (!window) {
        FERROR("SDL failed to create window: {}", SDL_GetError());
        SDL_Quit();
        return false;
    }

    // if (!SDL_Vulkan_LoadLibrary(NULL)) {
    //     FERROR("SDL failed to load vulkan : {}", SDL_GetError());
    //     SDL_DestroyWindow(window);
    //     SDL_Quit();
    //     return false;
    // }

    Uint32             extensionCount = 0;
    const char *const *extensions     = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
    if (!extensions) {
        FERROR("Failed to load extensions: {}", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    if (!gcontext.initialize(name, extensionCount, extensions, window)) {
        FINFO("Couldn't initialize graphics::GraphicsContext for main application.");
        return false;
    }

    VkExtent2D extent         = get_window_extent();
    gcontext.swapchain.extent = extent;
    gcontext.swapchain.create(gcontext, extent.width, extent.height);

    FInputManager.register_window(window);
    // SDL_ShowWindow(window);
    // SDL_RaiseWindow(window);

    FINFO("initialized SDL window successfully.");

    register_default_input_bindings();

    // IMGUI_CHECKVERSION();
    // ImGui::CreateContext();
    // ImGuiIO &io = ImGui::GetIO();
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking branch features
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Optional: allows floating windows outside main OS window
    // ImGui_ImplSDL3_InitForVulkan(window);               // window is your SDL_Window* pointer

    // VkDescriptorPoolSize       pool_sizes[] = { { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 } };
    // VkDescriptorPoolCreateInfo pool_info    = {};
    // pool_info.sType                         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    // pool_info.flags                         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    // pool_info.maxSets                       = 1000;
    // pool_info.poolSizeCount                 = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    // pool_info.pPoolSizes                    = pool_sizes;

    // // VkDescriptorPool imgui_pool;
    // vkCreateDescriptorPool(gcontext.vkb_device.device, &pool_info, nullptr, &imgui_pool);

    // ImGui_ImplVulkan_InitInfo init_info = {};
    // init_info.Instance                  = gcontext.vkb_instance.instance;
    // init_info.PhysicalDevice            = gcontext.vkb_device.physical_device;
    // init_info.Device                    = gcontext.vkb_device.device;
    // init_info.Queue                     = gcontext.graphics_queue; // Make sure you store your VkQueue handle!
    // init_info.DescriptorPool            = imgui_pool;
    // init_info.MinImageCount             = 2;
    // init_info.ImageCount                = 2;
    // // init_info.MSAASamples               = VK_SAMPLE_COUNT_1_BIT;

    // VkFormat format = VK_FORMAT_B8G8R8A8_SRGB;
    // // REQUIRED because you enabled dynamic rendering:
    // init_info.UseDynamicRendering                          = true;
    // init_info.PipelineInfoMain.PipelineRenderingCreateInfo = { .sType =
    //                                                                VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO
    //                                                                };
    // init_info.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    // init_info.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats =
    //     &format; // e.g., VK_FORMAT_B8G8R8A8_SRGB

    // ImGui_ImplVulkan_Init(&init_info);

    {
        std::lock_guard<std::mutex> lock(imgui_frame_mutex);
        imgui_payload.draw_data = nullptr;
        imgui_payload.ready     = false;
        imgui_shutting_down     = false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // Keep ViewportsEnable off for now while using Option A with render_thread.

    ImGui_ImplSDL3_InitForVulkan(window);

    // VkDescriptorPoolSize       pool_sizes[] = { { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 } };
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

    vkCreateDescriptorPool(gcontext.vkb_device.device, &pool_info, nullptr, &imgui_pool);

    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.ApiVersion          = VK_API_VERSION_1_4; // Match your Vulkan instance/device version
    init_info.Instance            = gcontext.vkb_instance.instance;
    init_info.PhysicalDevice      = gcontext.vkb_device.physical_device;
    init_info.Device              = gcontext.vkb_device.device;
    init_info.QueueFamily         = gcontext.graphics_family_index;
    init_info.Queue               = gcontext.graphics_queue;
    init_info.DescriptorPool      = imgui_pool;
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

    is_running      = true;
    is_game_running = true;
    game_thread     = std::jthread([this]() { game_thread_proc(); });
    render_thread   = std::jthread([this]() { render_thread_proc(); });
    FINFO("Game thread started.");
    FINFO("Application initialized successfully.");

    return true;
}

void
Application::register_default_input_bindings() {
    game_mouse_button_bindings.clear();
    game_key_bindings.clear();
    key_bindings.clear(); // just in case if we restart application.
    key_bindings.push_back(KeyBinding(Action::Quit,
                                      {},                                 // No modifiers
                                      input::KeyboardKeys::Value::ESCAPE, // <-- Use Value namespace
                                      [this]() { on_quit_triggered(); }));

    key_bindings.push_back(
        KeyBinding(Action::SaveSpecial,
                   { input::KeyboardKeys::Value::LCTRL, input::KeyboardKeys::Value::LSHIFT }, // <-- Use Value namespace
                   input::KeyboardKeys::Value::S,                                             // <-- Use Value namespace
                   []() { FINFO("Ctrl+Shift+S pressed."); }));

    key_bindings.push_back(KeyBinding(Action::Reopen, {}, input::KeyboardKeys::Value::F11, [this]() {
        reopen_requested = true;
        // shutdown();
        // init(application_name.c_str(), window_width, window_height);
        FINFO("Relaunching application...");
    }));
}

void
Application::on_quit_triggered() {
    FINFO("ESC key pressed - closing application...");
    is_running = false;
}

void
Application::handle_input() {
    triggered_actions_this_frame.clear();
    SDL_WindowID                        window_id = SDL_GetWindowID(window);

    std::vector<KeyBinding>             local_key_bindings;
    std::vector<GameKeyBinding>         local_game_key_bindings;
    std::vector<GameMouseButtonBinding> local_game_mouse_bindings;
    {
        std::lock_guard<std::mutex> lock(bindings_mutex);
        local_key_bindings        = key_bindings;
        local_game_key_bindings   = game_key_bindings;
        local_game_mouse_bindings = game_mouse_button_bindings;
    }

    // Store callbacks to run AFTER iteration finishes to prevent iterator invalidation
    std::vector<std::function<void()>> callbacks_to_run;

    // 1. handle key_bindings
    for (const auto &binding : local_key_bindings) {
        bool modifiers_met = true;
        for (auto mod : binding.required_down) {
            if (!FInputManager.is_key_down(window_id, mod)) {
                modifiers_met = false;
                break;
            }
        }

        if (modifiers_met && FInputManager.is_key_pressed(window_id, binding.pressed_this_frame)) {
            triggered_actions_this_frame.push_back(binding.action);
            if (binding.on_triggered) {
                callbacks_to_run.push_back(binding.on_triggered);
            }
        }
    }

    // 2. handle game-specific keybindings
    for (const auto &binding : local_game_key_bindings) {
        bool modifiers_met = true;
        for (auto mod : binding.required_down) {
            if (!FInputManager.is_key_down(window_id, mod)) {
                modifiers_met = false;
                break;
            }
        }

        if (modifiers_met && FInputManager.is_key_pressed(window_id, binding.pressed_this_frame)) {
            if (binding.on_triggered) {
                callbacks_to_run.push_back(binding.on_triggered);
            }
        }
    }

    // handle mouse button bindings.
    for (const auto &binding : local_game_mouse_bindings) {
        bool modifiers_met = true;
        for (auto mod : binding.required_down) {
            if (!FInputManager.is_key_down(window_id, mod)) {
                modifiers_met = false;
                break;
            }
        }

        if (modifiers_met && FInputManager.is_mouse_button_pressed(window_id, binding.button)) {
            if (binding.on_triggered) {
                callbacks_to_run.push_back(binding.on_triggered);
            }
        }
    }

    // Execute all collected callbacks safely *after* the loops have safely exited
    for (const auto &callback : callbacks_to_run) {
        callback();
    }
}

void
Application::game_thread_proc() {
    utils::FTimer game_timer;
    while (is_game_running.load(std::memory_order_relaxed)) {
        game_timer.begin();
        f32 dt = std::min(game_timer.dt(), 0.1f);

        components::update_script_component(dt);
        // Target tick rate (e.g., ~100Hz / 10ms)
        // std::this_thread::sleep_for(std::chrono::milliseconds(5));
        if (game_timer.end()) {
            game_dt.store(game_timer.dt_avg(), std::memory_order_relaxed);
            game_fps.store(game_timer.fps_avg(), std::memory_order_relaxed);
        }
    }
}

void
Application::render_thread_proc() {
    utils::FTimer render_timer;
    while (is_game_running.load(std::memory_order_relaxed)) {
        render_timer.begin();
        f32 dt = std::min(render_timer.dt(), 0.1f);

        render();
        // Target tick rate (e.g., ~100Hz / 10ms)
        // std::this_thread::sleep_for(std::chrono::milliseconds(5));
        if (render_timer.end()) {
            render_dt.store(render_timer.dt_avg(), std::memory_order_relaxed);
            render_fps.store(render_timer.fps_avg(), std::memory_order_relaxed);
        }
    }
}

void
Application::handle_resize() {
    FDEBUG("Hello from handle_resize()");
    VkExtent2D extent = get_window_extent();

    // Pause rendering if minimized
    if (extent.width == 0 || extent.height == 0) {
        is_minimized = true;
        return;
    }
    is_minimized  = false;

    window_width  = extent.width;
    window_height = extent.height;

    vkDeviceWaitIdle(gcontext.vkb_device.device);
    gcontext.recreate_swapchain(extent.width, extent.height);
    swapchain_resize_requested = false;

    FINFO("Resize handled.");
}

VkExtent2D
Application::get_window_extent() const {
    i32 w = 0, h = 0;
    SDL_GetWindowSizeInPixels(window, &w, &h);
    return VkExtent2D{ static_cast<u32>(w), static_cast<u32>(h) };
}

void
Application::process_sdl_events(const SDL_Event &event) {

    switch (event.type) {
    case SDL_EVENT_QUIT: {
        is_running = false;
    } break;

    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
        swapchain_resize_requested = true;
        FINFO("Swapchain resize requested.");
    } break;

    case SDL_EVENT_WINDOW_MINIMIZED: {
        is_minimized = true;
        FINFO("Window minimized");
    } break;

    case SDL_EVENT_WINDOW_RESTORED: {
        is_minimized               = false;
        swapchain_resize_requested = true;
        FINFO("Window restored");
    } break;

    default:
        break;
    }

    // feed events into our input manager
    FInputManager.handle_event(event);
}

void
Application::update(f32 dt) {
    FInputManager.prepare_for_new_frame();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);
        process_sdl_events(event);
    }
    handle_input();

    // Wait if previous ImGui frame hasn't been consumed by render_thread yet
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // Top bar + full-screen DockSpace (UE5-style editor shell)
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Reopen", "F11")) {
                reopen_requested = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Esc")) {
                is_running = false;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    ImGui::Begin("Viewport");
    // Later: place an ImGui::Image(...) here using a VkDescriptorSet from ImGui_ImplVulkan_AddTexture(...)
    ImGui::Text("3D Scene Viewport");
    ImGui::End();

    ImGui::Begin("Details");
    ImGui::Text("Selected Actor / Properties");
    ImGui::End();

    ImGui::Render();

    if (reopen_requested) {
        reopen_requested = false;
        FINFO("Relaunching application...");
        shutdown();
        init(application_name.c_str(), window_width, window_height);
    }

    // Publish ImGui::GetDrawData() to render_thread (guarded by mutex/cv)
    {
        std::unique_lock<std::mutex> lock(imgui_frame_mutex);
        // Wait until render_thread has finished using the prior frame's ImDrawData
        imgui_frame_cv.wait(lock, [] { return !imgui_payload.ready || imgui_shutting_down; });
        if (imgui_shutting_down) {
            return;
        }
        imgui_payload.draw_data = ImGui::GetDrawData();
        imgui_payload.ready     = (imgui_payload.draw_data != nullptr);
    }
    imgui_frame_cv.notify_one(); // make sure render thread can now read that data.

    // components::update_script_component(dt);
}

void
Application::render() {
    if (!begin_frame()) return;

    VkDevice       device           = gcontext.device;
    VkSwapchainKHR swapchain_handle = gcontext.swapchain.get_handle();
    if (swapchain_handle == VK_NULL_HANDLE) {
        end_frame();
        return;
    }

    // 1. Create a temporary fence for acquisition
    VkFenceCreateInfo fence_info{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    VkFence           acquire_fence;
    if (vkCreateFence(device, &fence_info, nullptr, &acquire_fence) != VK_SUCCESS) {
        end_frame();
        return;
    }

    u32      image_index = 0;
    VkResult result =
        vkAcquireNextImageKHR(device, swapchain_handle, 1000000000, VK_NULL_HANDLE, acquire_fence, &image_index);

    vkWaitForFences(device, 1, &acquire_fence, VK_TRUE, UINT64_MAX);
    vkDestroyFence(device, acquire_fence, nullptr);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        swapchain_resize_requested = true;
        FWARN("Acquired swapchain image on index {} was out of date or suboptimal.", image_index);
        end_frame();
        return;
    } else if (result != VK_SUCCESS) {
        FERROR("Failed to acquire swapchain image! VkResult: {}", static_cast<int>(result));
        end_frame();
        return;
    }

    VkImage     image      = gcontext.swapchain.get_images()[image_index];
    VkImageView image_view = gcontext.swapchain.get_image_views()[image_index];
    VkExtent2D  extent     = gcontext.swapchain.get_extent();

    // 2. Allocate a temporary command buffer to do the clear and layout transition
    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags            = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    pool_info.queueFamilyIndex = gcontext.graphics_family_index;

    VkCommandPool temp_pool;
    vkCreateCommandPool(device, &pool_info, nullptr, &temp_pool);

    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool        = temp_pool;
    alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(device, &alloc_info, &cmd);

    // 3. Record commands (Transition -> Clear via Dynamic Rendering -> Transition)
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &begin_info);

    // --- BARRIER 1: UNDEFINED -> COLOR_ATTACHMENT_OPTIMAL ---
    VkImageMemoryBarrier2 barrier_to_color{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
    barrier_to_color.srcStageMask        = VK_PIPELINE_STAGE_2_NONE;
    barrier_to_color.srcAccessMask       = 0;
    barrier_to_color.dstStageMask        = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    barrier_to_color.dstAccessMask       = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    barrier_to_color.oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier_to_color.newLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier_to_color.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier_to_color.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier_to_color.image               = image;
    barrier_to_color.subresourceRange    = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    VkDependencyInfo dep_info_to_color{ .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
    dep_info_to_color.imageMemoryBarrierCount = 1;
    dep_info_to_color.pImageMemoryBarriers    = &barrier_to_color;
    vkCmdPipelineBarrier2(cmd, &dep_info_to_color);

    // --- DYNAMIC RENDERING: CLEAR SCREEN ---
    VkRenderingAttachmentInfo color_attachment{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
    color_attachment.imageView   = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;    // Clears the screen
    color_attachment.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;   // Stores it to present
    color_attachment.clearValue  = { { 0.1f, 0.2f, 0.4f, 1.0f } }; // Blueish clear color

    VkRenderingInfo rendering_info{ .sType = VK_STRUCTURE_TYPE_RENDERING_INFO };
    rendering_info.renderArea           = { { 0, 0 }, extent };
    rendering_info.layerCount           = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments    = &color_attachment;

    // remember to keep vkCmdBeginRendering and vkCmdEndRendering when working around imgui.
    ImDrawData *frame_draw_data = nullptr;
    {
        std::lock_guard<std::mutex> lock(imgui_frame_mutex);
        if (imgui_payload.ready) {
            frame_draw_data = imgui_payload.draw_data;
        }
    }

    vkCmdBeginRendering(cmd, &rendering_info);
    if (frame_draw_data != nullptr && frame_draw_data->CmdListsCount > 0) {
        ImGui_ImplVulkan_RenderDrawData(frame_draw_data, cmd);
    }

    vkCmdEndRendering(cmd);

    // --- BARRIER 2: COLOR_ATTACHMENT_OPTIMAL -> PRESENT_SRC_KHR ---
    VkImageMemoryBarrier2 barrier_to_present{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
    barrier_to_present.srcStageMask        = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    barrier_to_present.srcAccessMask       = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    barrier_to_present.dstStageMask        = VK_PIPELINE_STAGE_2_NONE;
    barrier_to_present.dstAccessMask       = 0;
    barrier_to_present.oldLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier_to_present.newLayout           = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier_to_present.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier_to_present.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier_to_present.image               = image;
    barrier_to_present.subresourceRange    = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    VkDependencyInfo dep_info_to_present{ .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
    dep_info_to_present.imageMemoryBarrierCount = 1;
    dep_info_to_present.pImageMemoryBarriers    = &barrier_to_present;
    vkCmdPipelineBarrier2(cmd, &dep_info_to_present);

    vkEndCommandBuffer(cmd);

    // 4. Submit the command buffer
    VkSubmitInfo2             submit_info{ .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
    VkCommandBufferSubmitInfo cmd_info{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
    cmd_info.commandBuffer             = cmd;
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos    = &cmd_info;

    VkFenceCreateInfo submit_fence_info{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    VkFence           submit_fence;
    vkCreateFence(device, &submit_fence_info, nullptr, &submit_fence);

    vkQueueSubmit2(gcontext.graphics_queue, 1, &submit_info, submit_fence);
    vkWaitForFences(device, 1, &submit_fence, VK_TRUE, UINT64_MAX);

    // notify that imgui has been submitted to rendering.
    {
        std::lock_guard<std::mutex> lock(imgui_frame_mutex);
        imgui_payload.draw_data = nullptr;
        imgui_payload.ready     = false;
    }
    imgui_frame_cv.notify_all();

    vkDestroyFence(device, submit_fence, nullptr);
    vkDestroyCommandPool(device, temp_pool, nullptr);

    // 5. Present the image
    VkPresentInfoKHR present_info{ .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    present_info.swapchainCount = 1;
    present_info.pSwapchains    = &swapchain_handle;
    present_info.pImageIndices  = &image_index;

    VkResult present_result     = vkQueuePresentKHR(gcontext.present_queue, &present_info);
    if (present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR) {
        swapchain_resize_requested = true;
        FWARN("Presenting swapchain for image index: {} failed or suboptimal. Requesting resize...", image_index);
    }

    end_frame();
}

bool
Application::begin_frame() {
    if (is_minimized) return false;
    if (swapchain_resize_requested) {
        handle_resize();
        if (is_minimized) return false; // just in case minimization happened during resize.
    }

    if (gcontext.swapchain.get_handle() == VK_NULL_HANDLE) return false;
    // if (swapchain_resize_requested && !is_minimized) {
    //     i32 width, height;
    //     SDL_GetWindowSizeInPixels(window, &width, &height);
    //     if (width > 0 && height > 0) {
    //         FDEBUG("TODO: recreate or request renderer to recreate a swapchain. ({}, {})", width, height);
    //         // swapchain_resize_requested = false;
    //         handle_resize();
    //     }
    // }
    return true;
}

void
Application::end_frame() {}

void
Application::shutdown() {
    is_game_running.store(false);

    // make sure to notify render thread that imgui is no longer required to render.
    {
        std::lock_guard<std::mutex> lock(imgui_frame_mutex);
        imgui_shutting_down = true;
    }
    imgui_frame_cv.notify_all();

    if (render_thread.joinable()) render_thread.join();
    if (game_thread.joinable()) {
        game_thread.join();
    }

    vkDeviceWaitIdle(gcontext.vkb_device.device);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(gcontext.vkb_device.device, imgui_pool, nullptr);

    components::shutdown_script_component_system();

    // Clean up in reverse creation order

    gcontext.destroy();

    FInputManager.unregister_window(window); // SDL_GetWindowFromID(window_id)
    SDL_DestroyWindow(window);
    // SDL_Vulkan_UnloadLibrary();
    SDL_Quit();

    FINFO("Closed SDL window.");

    shutdown_logging();
}

void
Application::run() {
    if (!init(application_name.c_str(), window_width, window_height)) {
        FERROR("Application initialization failed. Exiting.");
        return;
    }
    while (is_running) {
        timer.begin();
        // std::this_thread::sleep_for(std::chrono::milliseconds(10));
        update(timer.dt());
        // render();

        // if (timer.end()) set_sdl_window_title_frame_metrics(window, application_name, timer.dt_avg(),
        // timer.fps_avg());
        if (timer.end()) {
            set_sdl_window_title_frame_metrics(
                window, application_name, game_fps.load(), render_dt.load(), render_fps.load());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    shutdown();
}

} // namespace ff