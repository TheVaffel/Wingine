#pragma once

#include <vulkan/vulkan.hpp>

namespace wg::internal::memoryUtil {
    vk::DeviceMemory createAndBindMemoryForImage(const vk::Image& image,
                                                 const vk::MemoryPropertyFlagBits& memory_properties,
                                                 const vk::Device& device,
                                                 const vk::PhysicalDeviceMemoryProperties& device_properties);

    vk::DeviceMemory createAndBindHostAccessibleMemoryForImage(
        const vk::Image& image,
        const vk::Device& device,
        const vk::PhysicalDeviceMemoryProperties& device_properties);

    vk::DeviceMemory createAndBindMemoryForBuffer(
        const vk::Buffer& buffer,
        const vk::MemoryPropertyFlags& memory_properties,
        const vk::Device& device,
        const vk::PhysicalDeviceMemoryProperties& device_properties);

    vk::DeviceMemory createAndBindHostAccessibleMemoryForBuffer(
        const vk::Buffer& buffer,
        const vk::Device& device,
        const vk::PhysicalDeviceMemoryProperties& device_properties);


    vk::MappedMemoryRange getMappedMemoryRangeForCopy(uint32_t copy_size,
                                                      uint32_t offset,
                                                      uint32_t alignment_requirement,
                                                      uint32_t allocated_memory_byte_size,
                                                      const vk::DeviceMemory& memory);

    template<typename T>
    T* mapMemory(const vk::DeviceMemory& memory,
                 const vk::Device& device);

    void unmapMemory(const vk::DeviceMemory& memory,
                     const vk::Device& device);
};

#include "./memoryUtil.impl.hpp"
