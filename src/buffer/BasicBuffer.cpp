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

    BasicBuffer::BasicBuffer(uint32_t byte_size,
                             const vk::BufferUsageFlags& usage,
                             const vk::MemoryPropertyFlags& memory_properties,
                             std::shared_ptr<const DeviceManager> device_manager)
        : byte_size(byte_size), device_manager(device_manager) {
        this->buffer = bufferUtil::createBuffer(byte_size, usage, device_manager->getDevice());
        this->memory = memoryUtil::createAndBindMemoryForBuffer(this->buffer,
                                                                memory_properties,
                                                                device_manager->getDevice(),
                                                                device_manager->getDeviceMemoryProperties());
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

    uint32_t BasicBuffer::getAllocatedByteSize() const {
        return this->device_manager->getDevice().getBufferMemoryRequirements(this->buffer).size;
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
