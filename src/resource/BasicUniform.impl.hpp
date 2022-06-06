#pragma once

#include "./BasicUniform.hpp"

#include "../buffer/BasicBuffer.hpp"
#include "./ResourceWriteBufferAuxillaryData.hpp"

#include "../memory/memoryUtil.hpp"

namespace wg::internal {

    template <typename T>
    BasicUniform<T>::BasicUniform(std::shared_ptr<const DeviceManager> device_manager)
        : device_manager(device_manager) {

        this->uniform_buffer = std::make_unique<BasicBuffer>(sizeof(T),
                                                             vk::BufferUsageFlagBits::eUniformBuffer,
                                                             vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                                                             device_manager);
    }

    template<typename T>
    void BasicUniform<T>::set(const T& val) {
        T* dst = memoryUtil::mapMemory<T>(this->uniform_buffer->getMemory(),
                                          this->device_manager->getDevice());

        memcpy(dst, &val, sizeof(T));
        memoryUtil::unmapMemory(this->uniform_buffer->getMemory(),
                                this->device_manager->getDevice());
    }

    template <typename T>
    std::unique_ptr<IResourceWriteAuxillaryData>
    BasicUniform<T>::writeDescriptorUpdate(vk::WriteDescriptorSet& write_info) const {
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
