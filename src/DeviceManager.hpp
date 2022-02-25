#pragma once

#include <vulkan/vulkan.hpp>

#include <memory>

#include "./VulkanInstanceManager.hpp"

namespace wg::internal {

    class DeviceManager {
        vk::Device device;
        vk::PhysicalDevice physical_device;
        vk::PhysicalDeviceMemoryProperties device_memory_props;

        std::shared_ptr<VulkanInstanceManager> vulkan_instance_manager;

    public:

        DeviceManager(std::shared_ptr<VulkanInstanceManager> vulkan_instance_manager);
        ~DeviceManager();

        const vk::Device getDevice() const;
        const vk::PhysicalDevice getPhysicalDevice() const;
        const vk::PhysicalDeviceMemoryProperties getDeviceMemoryProperties() const;
    };
};
