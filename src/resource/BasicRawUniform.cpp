#include "./BasicRawUniform.hpp"

#include "../buffer/BasicBuffer.hpp"
#include "./ResourceWriteBufferAuxillaryData.hpp"

#include "../memory/memoryUtil.hpp"

namespace wg::internal {

    BasicRawUniform::BasicRawUniform(uint32_t byte_size, std::shared_ptr<const DeviceManager> device_manager)
        : byte_size(byte_size), device_manager(device_manager) {

        this->uniform_buffer = std::make_unique<BasicBuffer>(byte_size,
                                                             vk::BufferUsageFlagBits::eUniformBuffer,
                                                             vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                                                             device_manager);
    }

    void BasicRawUniform::set(const void* ptr) {
        vk::MappedMemoryRange range;
        range.setMemory(this->uniform_buffer->getMemory())
            .setOffset(0)
            .setSize(VK_WHOLE_SIZE);

        void* dst = memoryUtil::mapRawMemory(this->uniform_buffer->getMemory(),
                                          this->device_manager->getDevice());

        memcpy(dst, ptr, this->byte_size);

        this->device_manager->getDevice().flushMappedMemoryRanges({range});

        memoryUtil::unmapMemory(this->uniform_buffer->getMemory(),
                                this->device_manager->getDevice());
    }

    uint32_t BasicRawUniform::getByteSize() const {
        return this->byte_size;
    }

    std::unique_ptr<IResourceWriteAuxillaryData>
    BasicRawUniform::writeDescriptorUpdate(vk::WriteDescriptorSet& write_info) const {
        auto aux_data = std::make_unique<ResourceWriteBufferAuxillaryData>();
        aux_data->buffer_info.buffer = this->uniform_buffer->getBuffer();
        aux_data->buffer_info.offset = 0;
        aux_data->buffer_info.range = this->uniform_buffer->getByteSize();

        write_info
            .setDescriptorCount(1)
            .setDescriptorType(vk::DescriptorType::eUniformBuffer)
            .setPBufferInfo(&aux_data->buffer_info)
            .setDstArrayElement(0);
        return aux_data;
    }
};
