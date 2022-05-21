#include "./BasicStorageBuffer.hpp"

#include "../resource/ResourceWriteBufferAuxillaryData.hpp"

#include "./bufferUtil.hpp"
#include "../memory/memoryUtil.hpp"

namespace wg::internal {

    BasicStorageBuffer::BasicStorageBuffer(uint32_t byte_size,
                                           std::shared_ptr<const DeviceManager> device_manager)
        : BasicBuffer(byte_size,
                      vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
                      vk::MemoryPropertyFlagBits::eDeviceLocal,
                      device_manager)

    { }

    std::unique_ptr<IResourceWriteAuxillaryData>
    BasicStorageBuffer::writeDescriptorUpdate(vk::WriteDescriptorSet& write_info) const {
        auto aux_data = std::make_unique<ResourceWriteBufferAuxillaryData>();
        aux_data->buffer_info.buffer = this->BasicBuffer::getBuffer();
        aux_data->buffer_info.offset = 0;
        aux_data->buffer_info.range = this->BasicBuffer::getByteSize();

        write_info
            .setDescriptorCount(1)
            .setDescriptorType(vk::DescriptorType::eStorageBuffer)
            .setPBufferInfo(&aux_data->buffer_info)
            .setDstArrayElement(0);
        return aux_data;
    }

};
