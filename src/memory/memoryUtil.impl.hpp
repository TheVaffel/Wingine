#pragma once

#include "./memoryUtil.hpp"

namespace wg::internal::memoryUtil {
    template<typename T>
    T* mapMemory(const vk::DeviceMemory& memory,
                 const vk::Device& device) {
        return reinterpret_cast<T*>(device.mapMemory(memory, 0, VK_WHOLE_SIZE));
    }
};
