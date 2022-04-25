#include "./VulkanInstanceManager.hpp"

#include <vector>
#include <iostream>

namespace wg::internal {

    namespace {

#ifdef DEBUG
        /*
         * Debug callback
         */
        static VKAPI_ATTR VkBool32 VKAPI_CALL
        _debugCallbackFun(VkDebugReportFlagsEXT flags,
                          VkDebugReportObjectTypeEXT objectType,
                          uint64_t object,
                          size_t location, int32_t messageCode,
                          const char* pLayerPrefix, const char* pMessage,
                          void* pUserData) {

            std::cout << "[" << pLayerPrefix << "] Message: " << pMessage << std::endl;

            return false;
        }
#endif // DEBUG

        std::vector<const char*> getInstanceExtensionNames() {
            return std::vector<const char*> {
              VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME,
                  /* Platform specific extensions */
#ifndef HEADLESS
#ifdef WIN32
                VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#else // WIN32
                VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#endif // WIN32
#endif // HEADLESS
                /* Debug specific extensions */
#ifdef DEBUG
                VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
                VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
            };
        }

        std::vector<const char*> getInstanceLayerNames() {
            return std::vector<const char*> {
#ifdef DEBUG
                "VK_LAYER_LUNARG_standard_validation",
                // "VK_LAYER_LUNARG_api_dump",
                // "VK_LAYER_LUNARG_device_simulation",
                // "VK_LAYER_LUNARG_monitor",
                // "VK_LAYER_RENDERDOC_Capture",
                // "VK_LAYER_LUNARG_api_dump",
                // "VK_LAYER_LUNARG_object_tracker",
                // "VK_LAYER_LUNARG_screenshot",
                // "VK_LAYER_LUNARG_standard_validation",
                // "VK_LAYER_LUNARG_starter_layer",
                // "VK_LAYER_LUNARG_parameter_validation",
                // "VK_LAYER_GOOGLE_unique_objects",
                // "VK_LAYER_LUNARG_vktrace",
                "VK_LAYER_KHRONOS_validation",
                // "VK_LAYER_GOOGLE_threading",
#endif // DEBUG
            };
        }

#ifndef HEADLESS
        vk::SurfaceKHR getSurfaceFromVisual(vk::Instance instance,
                                            VisualHandleT0 arg0,
                                            VisualHandleT1 arg1) {
#ifdef WIN32
            vk::Win32SurfaceCreateInfoKHR info;
            info.setHinstance(arg0).setHwnd(arg1);

            return instance.createWin32SurfaceKHR(info, nullptr);
#else // WIN32
            vk::XlibSurfaceCreateInfoKHR info;
            info.setWindow(arg0).setDpy(arg1);

            return instance.createXlibSurfaceKHR(info, nullptr);
#endif // WIN32
        }
#endif // HEADLESS
    };

    void VulkanInstanceManager::init_instance(const std::string& application_name) {

        auto instance_extension_names = getInstanceExtensionNames();
        auto instance_layer_names = getInstanceLayerNames();

        uint32_t extension_count;
        vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
        std::vector<VkExtensionProperties> props(extension_count);
        vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, props.data() + 0);
        /* for (unsigned int i = 0; i < extension_count; i++) {
           std::cout << "Available extension: " << props[i].extensionName << std::endl;
           }

           for (unsigned int i = 0; i < instance_extension_names.size(); i++) {
           std::cout << "Tried extension: " << instance_extension_names[i] << std::endl;
           } */

        uint32_t layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

        std::vector<VkLayerProperties> availableLayers(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, availableLayers.data());

        /* for (VkLayerProperties& vv : availableLayers) {
           std::cout << "Available layer: " << vv.layerName << std::endl;
           }

           for (unsigned int i = 0; i < instance_layer_names.size(); i++) {
           std::cout << "Tried layer: " << instance_layer_names[i] << std::endl;
           } */

        vk::ApplicationInfo appInfo;
        appInfo
            .setPApplicationName(application_name.c_str())
            .setApplicationVersion(1)
            .setPEngineName("Wingine").setEngineVersion(1)
            .setApiVersion(VK_API_VERSION_1_2);

        vk::InstanceCreateInfo cInfo;
        cInfo.setPApplicationInfo(&appInfo).setEnabledExtensionCount(instance_extension_names.size())
            .setPpEnabledExtensionNames(instance_extension_names.data())
            .setEnabledLayerCount(instance_layer_names.size())
            .setPpEnabledLayerNames(instance_layer_names.size() ?
                                    instance_layer_names.data() :
                                    NULL);

        this->instance = vk::createInstance(cInfo);
    }

    void VulkanInstanceManager::init_dispatcher() {

        vk::DynamicLoader dl;
        PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
        this->dispatcher.init(vkGetInstanceProcAddr);
        this->dispatcher.init(this->instance);


    }

#ifdef DEBUG
    void VulkanInstanceManager::init_debug_callback() {
        vk::DebugReportCallbackCreateInfoEXT callbackInfo;
        callbackInfo.setFlags(vk::DebugReportFlagBitsEXT::eError |
                              vk::DebugReportFlagBitsEXT::eWarning |
                              vk::DebugReportFlagBitsEXT::ePerformanceWarning)
            .setPfnCallback(&_debugCallbackFun);

        this->debug_callback = this->instance
            .createDebugReportCallbackEXT(callbackInfo, nullptr, this->dispatcher);

    }
#endif // DEBUG


#ifndef HEADLESS
    void VulkanInstanceManager::init_surface(VisualHandleT0 v0,
                                             VisualHandleT1 v1) {
        this->surface = getSurfaceFromVisual(this->instance, v0, v1);
    }

    VulkanInstanceManager::VulkanInstanceManager(VisualHandleT0 visualValue0,
                                                 VisualHandleT1 visualValue1,
                                                 const std::string& application_name) {
        this->init_instance(application_name);
        this->init_dispatcher();
#ifdef DEBUG
        this->init_debug_callback();
#endif
        this->init_surface(visualValue0, visualValue1);
    }
#endif // HEADLESS


    VulkanInstanceManager::VulkanInstanceManager(const std::string& application_name) {
        this->init_instance(application_name);
        this->init_dispatcher();
#ifdef DEBUG
        this->init_debug_callback();
#endif
    }

    vk::Instance VulkanInstanceManager::getInstance() {
        return this->instance;
    }

    vk::DispatchLoaderDynamic VulkanInstanceManager::getDispatcher() {
        return this->dispatcher;
    }

    bool VulkanInstanceManager::hasSurface() const {
        return this->surface != vk::SurfaceKHR {};
    }

    vk::SurfaceKHR VulkanInstanceManager::getSurface() const {
        if (!this->hasSurface()) {
            throw std::runtime_error("[VulkanInstanceManager] Does not have valid surface");
        }

        return this->surface;
    }

    VulkanInstanceManager::~VulkanInstanceManager() {

        this->instance.destroy(this->surface);

#ifdef DEBUG
        this->instance.destroyDebugReportCallbackEXT(this->debug_callback, nullptr, this->dispatcher);
#endif // DEBUG

        this->instance.destroy();
    }

};
