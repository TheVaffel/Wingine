#pragma once

#include "./IRawUniform.hpp"
#include "../core/DeviceManager.hpp"
#include "../buffer/IBuffer.hpp"

namespace wg::internal {
    class BasicRawUniform : public IRawUniform {

        uint32_t byte_size;

        std::unique_ptr<IBuffer> uniform_buffer;
        std::shared_ptr<const DeviceManager> device_manager;

    public:

        BasicRawUniform(uint32_t byte_size, std::shared_ptr<const DeviceManager> device_manager);
        virtual ~BasicRawUniform() = default;

        virtual void set(const void* ptr) override;
        virtual uint32_t getByteSize() const override;

        virtual std::unique_ptr<IResourceWriteAuxillaryData>
        writeDescriptorUpdate(vk::WriteDescriptorSet& write_info) const override;
    };
};
