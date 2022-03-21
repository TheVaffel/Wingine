#pragma once

#include <vulkan/vulkan.hpp>

namespace wg::internal::memoryUtil {
    vk::DeviceMemory createAndBindMemoryForImage(const vk::Image& image,
                                                 const vk::Device& device,
                                                 const vk::PhysicalDeviceMemoryProperties& device_properties);
};
