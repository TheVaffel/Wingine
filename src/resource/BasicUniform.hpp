#pragma once

#include "./IUniform.hpp"
#include "../buffer/IBuffer.hpp"
#include "./BasicRawUniform.hpp"

#include "../core/DeviceManager.hpp"

namespace wg::internal {
    template <typename T>
    class BasicUniform : public IUniform<T> {

        BasicRawUniform raw_uniform;

    public:

        BasicUniform(std::shared_ptr<const DeviceManager> device_manager);
        virtual ~BasicUniform() = default;

        virtual void set(const T& val) override;

        virtual std::unique_ptr<IResourceWriteAuxillaryData>
        writeDescriptorUpdate(vk::WriteDescriptorSet& write_info) const override;
    };
};

#include "./BasicUniform.impl.hpp"
