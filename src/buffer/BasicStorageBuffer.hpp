#pragma once

#include "./IStorageBuffer.hpp"
#include "./BasicBuffer.hpp"

namespace wg::internal {
    class BasicStorageBuffer : public IStorageBuffer, public virtual BasicBuffer {
    public:

        virtual std::unique_ptr<IResourceWriteAuxillaryData>
        writeDescriptorUpdate(vk::WriteDescriptorSet& write_info) const override;

        BasicStorageBuffer(uint32_t byte_size,
                           std::shared_ptr<const DeviceManager> device_manager);
    };
};
