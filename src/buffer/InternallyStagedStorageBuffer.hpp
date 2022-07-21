#pragma once

#include "./IStorageBuffer.hpp"
#include "./BasicBuffer.hpp"

#include "./StagingBuffer.hpp"

namespace wg::internal {
    template<typename T>
    class InternallyStagedStorageBuffer : public IStorageBuffer<T>, public virtual BasicBuffer {

        StagingBuffer staging_buffer;
    public:

        virtual std::unique_ptr<IResourceWriteAuxillaryData>
        writeDescriptorUpdate(vk::WriteDescriptorSet& write_info) const override;

        InternallyStagedStorageBuffer(uint32_t element_count,
                                      std::shared_ptr<DeviceManager> device_manager,
                                      std::shared_ptr<QueueManager> queue_manager,
                                      std::shared_ptr<CommandManager> command_manager);

        virtual void set(const T* data, uint32_t first_element, uint32_t element_count) final;
    };
};

#include "./InternallyStagedStorageBuffer.impl.hpp"
