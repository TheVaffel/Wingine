#pragma once

#include <vulkan/vulkan.hpp>

namespace wg::internal::bufferUtil {
    vk::Buffer createHostVisibleBuffer(uint32_t byte_size, const vk::Device& device);
};
