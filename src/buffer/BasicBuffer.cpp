#include "./BasicBuffer.hpp"

#include "./bufferUtil.hpp"
#include "../memory/memoryUtil.hpp"

namespace wg::internal {
    BasicBuffer::BasicBuffer(uint32_t byte_size,
                             const vk::Buffer& buffer,
                             const vk::DeviceMemory& memory,
                             std::shared_ptr<const DeviceManager> device_manager)
        : buffer(buffer), memory(memory), byte_size(byte_size), device_manager(device_manager) {

    }

    std::unique_ptr<BasicBuffer>
    BasicBuffer::createHostVisibleBuffer(uint32_t byte_size,
                            std::shared_ptr<const DeviceManager> device_manager) {
        const vk::Device& device = device_manager->getDevice();

        vk::Buffer buffer = bufferUtil::createHostVisibleBuffer(byte_size, device);
        vk::DeviceMemory memory = memoryUtil::createAndBindHostAccessibleMemoryForBuffer(
            buffer,
            device,
            device_manager->getDeviceMemoryProperties());

        return std::unique_ptr<BasicBuffer>(new BasicBuffer(
                                                byte_size,
                                                buffer,
                                                memory,
                                                device_manager));
    }

    uint32_t BasicBuffer::getByteSize() const {
        return this->byte_size;
    }

    vk::Buffer BasicBuffer::getBuffer() const {
        return this->buffer;
    }

    vk::DeviceMemory BasicBuffer::getMemory() const {
        return this->memory;
    }

    BasicBuffer::~BasicBuffer() {
        this->device_manager->getDevice().destroy(this->buffer);
        this->device_manager->getDevice().free(this->memory);
    }
};
