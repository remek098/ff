#pragma once

#include "imgui.h"
#include "common/common_headers.h"
#include <mutex>

// forward declerations
struct SDL_Window;
namespace ff::graphics {
struct GraphicsContext;
}

namespace ff::editor {
struct EditorFramePayload {
    ImDrawData *draw_data{ nullptr }; // Valid until main thread calls NewFrame() again
    bool        ready{ false };
};

class ImGuiEditorElement {
  public:
    virtual ~ImGuiEditorElement() = default;
    virtual void update()         = 0;
};

class LambdaEditorElement : public ImGuiEditorElement {
  public:
    using DrawFn = std::function<void()>;

    explicit LambdaEditorElement(DrawFn fn) : fn_(std::move(fn)) {}

    void update() override {
        if (fn_) {
            fn_();
        }
    }

  private:
    DrawFn fn_;
};

using ElementId = std::size_t;
constexpr ElementId invalid_element_id{ 0 };

struct RegisteredElement {
    ElementId                    id;
    ff::uptr<ImGuiEditorElement> element;
};

// supposed to have widgets/elements recorded to by main thread and have it read/used by render thread.
class ImGuiLayer {
  public:
    DISABLE_COPY_AND_MOVE(ImGuiLayer);

    static ImGuiLayer &get() {
        static ImGuiLayer instance;
        return instance;
    }

    void      initialize(SDL_Window *window, graphics::GraphicsContext &gcontext);
    void      shutdown(graphics::GraphicsContext &gcontext);

    void      update();

    ElementId register_element(ff::uptr<ImGuiEditorElement> element);
    void      unregister_element(ElementId id);
    void      clear_elements();

    /// @brief Must be put between vkCmdBeginRendering and vkCmdEndRendering
    /// @param cmd VkCommandBuffer used for rendering ImGui with ImGui_ImplVulkan_RenderDrawData
    void render_draw_data(VkCommandBuffer cmd);
    void cleanup_payload();
    void stop_rendering();

    using DockspaceInitFn = std::function<void(ImGuiID dockspace_id, ImVec2 viewport_size)>;
    void set_dockspace_init_callback(DockspaceInitFn fn);

  private:
    ImGuiLayer()  = default;
    ~ImGuiLayer() = default;

  private:
    // just stores function handles/pointers that tell what to draw.
    std::vector<RegisteredElement> elements_to_draw_;
    std::mutex                     elements_mutex_;

    std::mutex                     frame_mutex_;
    std::condition_variable        frame_cv_;

    EditorFramePayload             payload_{ nullptr };
    VkDescriptorPool               imgui_pool_;

    ElementId                      next_element_id{ 1 }; // Start at 1 so 0 can mean "invalid / not registered"
    bool                           shutting_down_{ false };

    DockspaceInitFn                dockspace_init_fn_;
    bool                           dockspace_initialized_{ false };
};
} // namespace ff::editor

#define FImGuiLayer ff::editor::ImGuiLayer::get()