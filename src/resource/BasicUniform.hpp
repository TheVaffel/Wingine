#pragma once

#include "./IUniform.hpp"
#include "../buffer/IBuffer.hpp"

#include "../DeviceManager.hpp"

namespace wg::internal {
    template <typename T>
    class BasicUniform : public IUniform<T> {

        std::unique_ptr<IBuffer> uniform_buffer;
        std::shared_ptr<const DeviceManager> device_manager;

    public:

        BasicUniform(std::shared_ptr<const DeviceManager> device_manager);

        virtual void set(const T& val) override;

        virtual std::unique_ptr<IResourceWriteAuxillaryData>
        writeDescriptorUpdate(vk::WriteDescriptorSet& write_info) const override;

    };
};

#include "./BasicUniform.impl.hpp"
