#pragma once

#include <vulkan/vulkan.hpp>

namespace wg::internal::memoryUtil {
    vk::DeviceMemory createAndBindMemoryForImage(const vk::Image& image,
                                                 const vk::Device& device,
                                                 const vk::PhysicalDeviceMemoryProperties& device_properties);

    vk::DeviceMemory createAndBindHostAccessibleMemoryForImage(
        const vk::Image& image,
        const vk::Device& device,
        const vk::PhysicalDeviceMemoryProperties& device_properties);

    vk::DeviceMemory createAndBindHostAccessibleMemoryForBuffer(
        const vk::Buffer& buffer,
        const vk::Device& device,
        const vk::PhysicalDeviceMemoryProperties& device_properties);

    template<typename T>
    T* mapMemory(const vk::DeviceMemory& memory,
                 const vk::Device& device);

    void unmapMemory(const vk::DeviceMemory& memory,
                     const vk::Device& device);
};

#include "./memoryUtil.impl.hpp"
