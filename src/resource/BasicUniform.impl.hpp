#pragma once

#include "./BasicUniform.hpp"

#include "../buffer/BasicBuffer.hpp"
#include "./ResourceWriteBufferAuxillaryData.hpp"

namespace wg::internal {

    template <typename T>
    BasicUniform<T>::BasicUniform(std::shared_ptr<StagingBufferManager> staging_buffer_manager,
                                  std::shared_ptr<const DeviceManager> device_manager)
        : staging_buffer_manager(staging_buffer_manager) {
        this->underlying_buffer = std::make_unique<BasicBuffer>(sizeof(T),
                                                                vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst,
                                                                vk::MemoryPropertyFlagBits::eDeviceLocal,
                                                                device_manager);
    }

    template<typename T>
    void BasicUniform<T>::set(const T& val) {
        this->staging_buffer_manager->copySync(&val, *this->underlying_buffer, sizeof(T));
    }

    template <typename T>
    std::unique_ptr<IResourceWriteAuxillaryData>
    BasicUniform<T>::writeDescriptorUpdate(vk::WriteDescriptorSet& write_info) const {
        auto aux_data = std::make_unique<ResourceWriteBufferAuxillaryData>();
        aux_data->buffer_info.buffer = this->underlying_buffer->getBuffer();
        aux_data->buffer_info.offset = 0;
        aux_data->buffer_info.range = this->underlying_buffer->getByteSize();

        write_info
            .setDescriptorCount(1)
            .setDescriptorType(vk::DescriptorType::eUniformBuffer)
            .setPBufferInfo(&aux_data->buffer_info)
            .setDstArrayElement(0);
        return aux_data;
    }
};
