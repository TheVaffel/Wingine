#pragma once

#include "./BasicUniform.hpp"

#include "../buffer/BasicBuffer.hpp"
#include "./ResourceWriteBufferAuxillaryData.hpp"

#include "../memory/memoryUtil.hpp"

namespace wg::internal {

    template <typename T>
    BasicUniform<T>::BasicUniform(std::shared_ptr<const DeviceManager> device_manager)
        : raw_uniform(sizeof(T), device_manager) { }

    template<typename T>
    void BasicUniform<T>::set(const T& val) {
        this->raw_uniform.set(&val);
    }

    template <typename T>
    std::unique_ptr<IResourceWriteAuxillaryData>
    BasicUniform<T>::writeDescriptorUpdate(vk::WriteDescriptorSet& write_info) const {
        return this->raw_uniform.writeDescriptorUpdate(write_info);
    }
};
