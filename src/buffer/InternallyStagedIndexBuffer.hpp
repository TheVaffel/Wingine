#pragma once

#include "./IIndexBuffer.hpp"
#include "./BasicBuffer.hpp"
#include "./StagingBuffer.hpp"

#include "../core/CommandManager.hpp"

namespace wg::internal {
    class InternallyStagedIndexBuffer : public virtual IIndexBuffer, public BasicBuffer {
        StagingBuffer staging_buffer;

    public:

        InternallyStagedIndexBuffer(uint32_t num_indices,
                                    std::shared_ptr<DeviceManager> device_manager,
                                    std::shared_ptr<QueueManager> queue_manager,
                                    std::shared_ptr<CommandManager> command_manager);

        virtual void set(const uint32_t* data, uint32_t first_index, uint32_t num_indices) final;
    };
};
