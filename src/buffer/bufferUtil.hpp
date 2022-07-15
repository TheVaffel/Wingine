#pragma once

#include <vulkan/vulkan.hpp>

namespace wg::internal::bufferUtil {
    vk::Buffer createBuffer(uint32_t byte_size,
                            const vk::BufferUsageFlags& usage,
                            const vk::Device& device);

    vk::Buffer createStorageBuffer(uint32_t byte_size, const vk::Device& device);

    vk::Buffer createHostVisibleBuffer(uint32_t byte_size, const vk::Device& device);

    vk::MappedMemoryRange getMappedMemoryRangeForCopy(uint32_t copy_size,
                                                      uint32_t offset,
                                                      uint32_t alignment_requirement,
                                                      uint32_t allocated_memory_byte_size,
                                                      const vk::DeviceMemory& memory);
};
