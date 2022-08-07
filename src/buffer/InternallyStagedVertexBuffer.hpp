#pragma once

#include "./IVertexBuffer.hpp"
#include "./BasicBuffer.hpp"
#include "./StagingBuffer.hpp"

#include "../core/CommandManager.hpp"

namespace wg::internal {
    template<typename T>
    class InternallyStagedVertexBuffer : public virtual IVertexBuffer<T>, public BasicBuffer {
        StagingBuffer staging_buffer;

    public:

        InternallyStagedVertexBuffer(uint32_t num_elements,
                                     std::shared_ptr<DeviceManager> device_manager,
                                     std::shared_ptr<QueueManager> queue_manager,
                                     std::shared_ptr<CommandManager> command_manager);

        virtual void set(const T* data, uint32_t first_element, uint32_t element_count) final;
    };
};

#include "./InternallyStagedVertexBuffer.impl.hpp"
