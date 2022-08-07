#pragma once

#include "./IBuffer.hpp"
#include "../core/DeviceManager.hpp"

namespace wg::internal {
    class BasicBuffer : public virtual IBuffer {
        vk::Buffer buffer;
        vk::DeviceMemory memory;

        uint32_t byte_size;

        std::shared_ptr<const DeviceManager> device_manager;

        BasicBuffer(uint32_t byte_size,
                    const vk::Buffer& buffer,
                    const vk::DeviceMemory& memory,
                    std::shared_ptr<const DeviceManager> device_manager);

    public:
        BasicBuffer(uint32_t byte_size,
                    const vk::BufferUsageFlags& usage,
                    const vk::MemoryPropertyFlags& memory_properties,
                    std::shared_ptr<const DeviceManager> device_manager);

        static std::unique_ptr<BasicBuffer>
        createHostVisibleBuffer(uint32_t byte_size,
                                std::shared_ptr<const DeviceManager> device_manager);

        virtual uint32_t getByteSize() const;
        virtual uint32_t getAllocatedByteSize() const;
        virtual vk::Buffer getBuffer() const;
        virtual vk::DeviceMemory getMemory() const;

        ~BasicBuffer();
    };
};
