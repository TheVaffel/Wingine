#pragma once

#include <vulkan/vulkan.hpp>

namespace wg::internal::bufferUtil {
    vk::Buffer createBuffer(uint32_t byte_size,
                            const vk::BufferUsageFlags& usage,
                            const vk::Device& device);

    vk::Buffer createStorageBuffer(uint32_t byte_size, const vk::Device& device);

    vk::Buffer createHostVisibleBuffer(uint32_t byte_size, const vk::Device& device);
};
