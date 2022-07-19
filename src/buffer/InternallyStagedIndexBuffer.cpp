#include "./InternallyStagedIndexBuffer.hpp"

#include "../memory/memoryUtil.hpp"
#include "../sync/fenceUtil.hpp"
#include "./bufferUtil.hpp"
#include "./copyBuffer.hpp"

#include "../util/log.hpp"

namespace wg::internal {

    InternallyStagedIndexBuffer::InternallyStagedIndexBuffer(uint32_t num_indices,
                                                             std::shared_ptr<DeviceManager> device_manager,
                                                             std::shared_ptr<QueueManager> queue_manager,
                                                             std::shared_ptr<CommandManager> command_manager)
        : BasicBuffer(num_indices * sizeof(uint32_t),
                      vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                      vk::MemoryPropertyFlagBits::eDeviceLocal,
                      device_manager),
          staging_buffer(num_indices * sizeof(uint32_t),
                         device_manager,
                         queue_manager,
                         command_manager) { }

    void InternallyStagedIndexBuffer::set(const uint32_t* data, uint32_t first_index, uint32_t num_indices) {

        uint32_t copy_size = num_indices * sizeof(uint32_t);
        uint32_t offset = first_index * sizeof(uint32_t);

        this->staging_buffer.set(data, *this, copy_size, offset);
    }
};
