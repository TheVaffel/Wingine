#pragma once

#include <vulkan/vulkan.hpp>

namespace wg::internal {
    class IBuffer {
    public:
        virtual uint32_t getByteSize() const = 0;
        virtual uint32_t getAllocatedByteSize() const = 0;
        virtual vk::Buffer getBuffer() const = 0;
        virtual vk::DeviceMemory getMemory() const = 0;

        virtual ~IBuffer();
    };
};
