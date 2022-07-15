#pragma once

#include <vulkan/vulkan.hpp>

#include <memory>

#include "./VulkanInstanceManager.hpp"

namespace wg::internal {

    class DeviceManager {
        vk::Device device;
        vk::PhysicalDevice physical_device;
        vk::PhysicalDeviceProperties device_props;
        vk::PhysicalDeviceMemoryProperties device_memory_props;

        std::shared_ptr<VulkanInstanceManager> vulkan_instance_manager;

    public:

        DeviceManager(std::shared_ptr<VulkanInstanceManager> vulkan_instance_manager);
        ~DeviceManager();

        DeviceManager(const DeviceManager& deviceManager) = delete;

        const vk::Device& getDevice() const;
        const vk::PhysicalDevice& getPhysicalDevice() const;
        const vk::PhysicalDeviceMemoryProperties& getDeviceMemoryProperties() const;
        const vk::PhysicalDeviceProperties& getDeviceProperties() const;
    };
};
