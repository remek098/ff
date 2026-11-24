#pragma once

#include "core/common/common_headers.h"
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
// #include "../vendor/vkb/src/VkBootstrap.h"
#include "VkBootstrap.h"
#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/imgui_internal.h"
// #include "../vendor/sdl3/include/SDL3/SDL_main.h"

#include "core/logging/logger.h"
#include "core/input/input_system.h"

#include "core/application/application.h"
#include "core/application/imgui/imgui_layer.h"
#include "core/application/imgui/editor_elements/menu_bar.h"

#include "imgui_impl_vulkan.h"
#include "imgui_impl_sdl3.h"