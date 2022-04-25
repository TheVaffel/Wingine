#pragma once

#include "./IBuffer.hpp"
#include "../DeviceManager.hpp"

namespace wg::internal {
    class BasicBuffer : public IBuffer {
        vk::Buffer buffer;
        vk::DeviceMemory memory;

        uint32_t byte_size;

        std::shared_ptr<const DeviceManager> device_manager;

        BasicBuffer(uint32_t byte_size,
                    const vk::Buffer& buffer,
                    const vk::DeviceMemory& memory,
                    std::shared_ptr<const DeviceManager> device_manager);

    public:

        static std::unique_ptr<BasicBuffer>
        createHostVisibleBuffer(uint32_t byte_size,
                                std::shared_ptr<const DeviceManager> device_manager);

        virtual uint32_t getByteSize() const;
        virtual vk::Buffer getBuffer() const;
        virtual vk::DeviceMemory getMemory() const;

        ~BasicBuffer();
    };
};
