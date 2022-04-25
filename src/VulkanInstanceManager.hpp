#pragma once

#ifdef HEADLESS

#include <vulkan/vulkan.hpp>

#elif defined(WIN32)

#include <windows.h>

namespace wg {
    typedef HINSTANCE VisualHandleT0;
    typedef HWND VisualHandleT1;
};

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>
#include "vulkan/vulkan_win32.h"

#include <algorithm>

#undef min
#undef max

#else // HEADLESS || WIN32
#include <unistd.h>

#include <X11/Xlib.h>

namespace wg {
    typedef Window VisualHandleT0;
    typedef Display* VisualHandleT1;
};

#define VK_USE_PLATFORM_XLIB_KHR
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_xlib.h>

#endif // HEADLESS || WIN32

namespace wg::internal {

    class VulkanInstanceManager {

        vk::Instance instance;
        vk::SurfaceKHR surface = nullptr;
        vk::DispatchLoaderDynamic dispatcher;

#ifdef DEBUG
        vk::DebugReportCallbackEXT debug_callback;

        void init_debug_callback();
#endif // DEBUG

        void init_instance(const std::string& application_name);
        void init_dispatcher();
#ifndef HEADLESS
        void init_surface(VisualHandleT0 v0,
                          VisualHandleT1 v1);
#endif // HEADFUL

    public:

        vk::Instance getInstance();
        vk::DispatchLoaderDynamic getDispatcher();

        bool hasSurface() const;
        vk::SurfaceKHR getSurface() const;

        VulkanInstanceManager(const std::string& application_name);
#ifndef HEADLESS
        VulkanInstanceManager(VisualHandleT0 v0,
                              VisualHandleT1 v1,
                              const std::string& application_name);
#endif // HEADFUL

        ~VulkanInstanceManager();

        VulkanInstanceManager(const VulkanInstanceManager& vim) = delete;
    };
};
