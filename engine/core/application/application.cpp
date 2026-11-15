#include "application.h"
#include "components/script_component.h"
#include <format>

// #if BUILD_WITH_EDITOR
// #include "imgui.h"
// #include "imgui_impl_sdl3.h"
// #include "imgui_impl_vulkan.h"
// #endif

namespace ff {
VkDescriptorPool imgui_pool;

namespace {
void
set_sdl_window_title_frame_metrics(SDL_Window *window, const std::string &name, f32 dt, f32 fps) {
    const std::string title = std::format("{} | {:.2f} ms | {:.1f} FPS", name, dt * 1000.f, fps);

    SDL_SetWindowTitle(window, title.c_str());
}
} // namespace

Application::Application(const char *name, u32 width, u32 height)
    : application_name(name ? name : "ff"), window_width(width), window_height(height) { /*init(name, width, height);*/
}

namespace {
struct GameKeyBinding {
    std::string                             name;
    std::vector<input::KeyboardKeys::Value> required_down;
    input::KeyboardKeys::Value              pressed_this_frame;
    std::function<void()>                   on_triggered;
};

struct GameMouseButtonBinding {
    std::string                             name;
    std::vector<input::KeyboardKeys::Value> required_down; // e.g., Hold LSHIFT + Click
    input::MouseButton::Value               button;        // Left, Right, Middle, etc.
    std::function<void()>                   on_triggered;
};
std::vector<GameKeyBinding>         game_key_bindings;
std::vector<GameMouseButtonBinding> game_mouse_bindings;
} // namespace

void
Application::register_key_binding(std::string                             action_name,
                                  std::vector<input::KeyboardKeys::Value> modifiers,
                                  input::KeyboardKeys::Value              key,
                                  std::function<void()>                   callback) {
    FINFO("Registering game key binding: '{}'", action_name);
    game_key_bindings.push_back({ std::move(action_name), std::move(modifiers), key, std::move(callback) });
}

void
Application::register_mouse_binding(std::string                             action_name,
                                    std::vector<input::KeyboardKeys::Value> modifiers,
                                    input::MouseButton::Value               button,
                                    std::function<void()>                   callback) {
    FINFO("Registering game mouse binding: '{}'", action_name);
    game_mouse_bindings.push_back({ std::move(action_name), std::move(modifiers), button, std::move(callback) });
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

    if (!SDL_Vulkan_LoadLibrary(NULL)) {
        FERROR("SDL failed to load vulkan : {}", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

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

    VkExtent2D extent = get_window_extent();
    gcontext.swapchain.create(gcontext, gcontext.surface, extent.width, extent.height);

    FInputManager.register_window(window);
    // SDL_ShowWindow(window);
    // SDL_RaiseWindow(window);

    FINFO("initialized SDL window successfully.");
    is_running      = true;
    is_game_running = true;
    game_thread     = std::jthread([this]() { game_thread_proc(); });
    FINFO("Game thread started.");
    FINFO("Application initialized successfully.");

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
    //                                                                VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
    // init_info.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    // init_info.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats =
    //     &format; // e.g., VK_FORMAT_B8G8R8A8_SRGB

    // ImGui_ImplVulkan_Init(&init_info);

    return true;
}

void
Application::register_default_input_bindings() {
    game_mouse_bindings.clear();
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
        shutdown();
        init(application_name.c_str(), window_width, window_height);
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
    SDL_WindowID window_id = SDL_GetWindowID(window);

    // Store callbacks to run AFTER iteration finishes to prevent iterator invalidation
    std::vector<std::function<void()>> callbacks_to_run;

    // 1. handle key_bindings
    for (const auto &binding : key_bindings) {
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
    for (const auto &binding : game_key_bindings) {
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
    for (const auto &binding : game_mouse_bindings) {
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
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (game_timer.end()) {
            game_dt.store(game_timer.dt_avg(), std::memory_order_relaxed);
            game_fps.store(game_timer.fps_avg(), std::memory_order_relaxed);
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
    gcontext.swapchain.recreate(gcontext, gcontext.surface, extent.width, extent.height);
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
        // ImGui_ImplSDL3_ProcessEvent(&event);
        process_sdl_events(event);
    }
    handle_input();

    // components::update_script_component(dt);
}

void
Application::render() {
    if (swapchain_resize_requested && !is_minimized) {
        i32 width, height;
        SDL_GetWindowSizeInPixels(window, &width, &height);
        if (width > 0 && height > 0) {
            FDEBUG("TODO: recreate or request renderer to recreate a swapchain. ({}, {})", width, height);
            // swapchain_resize_requested = false;
            handle_resize();
        }
    }

    if (is_minimized) return;
    // // 1. Start the frame
    // ImGui_ImplVulkan_NewFrame();
    // ImGui_ImplSDL3_NewFrame();
    // ImGui::NewFrame();

    // // 2. Build your DockSpace UI layout
    // ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
    // ImGui::ShowDemoWindow(); // Example window

    // // 3. Render ImGui internal command structures
    // ImGui::Render();
    // ImDrawData *draw_data = ImGui::GetDrawData();

    // // 4. Record into your Vulkan command buffer (inside your render pass / dynamic rendering block)
    // // vkCmdBeginRenderPass(...) or vkCmdBeginRendering(...)
    // // ImGui_ImplVulkan_RenderDrawData(draw_data, command_buffer);
    // // vkCmdEndRenderPass(...) or vkCmdEndRendering(...)

    // // 5. Handle multi-viewports if enabled (renders floating editor windows)
    // ImGuiIO& io = ImGui::GetIO();
    // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    //     ImGui::UpdatePlatformWindows();
    //     ImGui::RenderPlatformWindowsDefault();
    // }
}

void
Application::shutdown() {
    is_game_running.store(false);
    if (game_thread.joinable()) {
        game_thread.join();
    }

    vkDeviceWaitIdle(gcontext.vkb_device.device);
    // ImGui_ImplVulkan_Shutdown();
    // ImGui_ImplSDL3_Shutdown();
    // ImGui::DestroyContext();
    // vkDestroyDescriptorPool(gcontext.vkb_device.device, imgui_pool, nullptr);

    components::shutdown_script_component_system();

    // Clean up in reverse creation order
    gcontext.swapchain.destroy(gcontext);
    if (gcontext.surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(gcontext.vkb_instance.instance, gcontext.surface, nullptr);
        gcontext.surface = VK_NULL_HANDLE;
    }
    gcontext.destroy();

    FInputManager.unregister_window(window); // SDL_GetWindowFromID(window_id)
    SDL_DestroyWindow(window);
    SDL_Vulkan_UnloadLibrary();
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
        render();

        // if (timer.end()) set_sdl_window_title_frame_metrics(window, application_name, timer.dt_avg(),
        // timer.fps_avg());
        if (timer.end()) {
            set_sdl_window_title_frame_metrics(window, application_name, game_dt.load(), game_fps.load());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    shutdown();
}

} // namespace ff