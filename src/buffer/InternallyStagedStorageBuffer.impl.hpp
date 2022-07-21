#pragma once

#include "./InternallyStagedStorageBuffer.hpp"

#include "../resource/ResourceWriteBufferAuxillaryData.hpp"

#include "./bufferUtil.hpp"
#include "../memory/memoryUtil.hpp"

namespace wg::internal {

    template <typename T>
    InternallyStagedStorageBuffer<T>::InternallyStagedStorageBuffer(uint32_t num_elements,
                                           std::shared_ptr<DeviceManager> device_manager,
                                           std::shared_ptr<QueueManager> queue_manager,
                                           std::shared_ptr<CommandManager> command_manager)
        : BasicBuffer(num_elements * sizeof(T),
                      vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
                      vk::MemoryPropertyFlagBits::eDeviceLocal,
                      device_manager),
          staging_buffer(num_elements * sizeof(T),
                         device_manager,
                         queue_manager,
                         command_manager)
    { }

    template <typename T>
    std::unique_ptr<IResourceWriteAuxillaryData>
    InternallyStagedStorageBuffer<T>::writeDescriptorUpdate(vk::WriteDescriptorSet& write_info) const {
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

    template <typename T>
    void InternallyStagedStorageBuffer<T>::set(const T* data, uint32_t first_element, uint32_t element_count) {
        uint32_t copy_size = element_count * sizeof(T);
        uint32_t offset = first_element * sizeof(T);
        this->staging_buffer.set(data, *this, copy_size, offset);
    }
};
