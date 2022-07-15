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
                         vk::BufferUsageFlagBits::eTransferSrc,
                         vk::MemoryPropertyFlagBits::eHostVisible,
                         device_manager),
          device_manager(device_manager),
          command_manager(command_manager) {
        this->command = this->command_manager->createGraphicsCommands(1)[0];
        this->graphics_queue = queue_manager->getGraphicsQueue();
    }

    void InternallyStagedIndexBuffer::set(const uint32_t* data, uint32_t first_index, uint32_t num_indices) {

        uint32_t copy_size = num_indices * sizeof(uint32_t);
        uint32_t offset = first_index * sizeof(uint32_t);

        uint32_t required_size_multiple = this->device_manager->getDeviceProperties().limits.nonCoherentAtomSize;

        uint32_t* dst = memoryUtil::mapMemory<uint32_t>(this->staging_buffer.getMemory(),
                                          this->device_manager->getDevice());

        memcpy(dst, data, sizeof(uint32_t) * num_indices);


        vk::MappedMemoryRange range = bufferUtil::getMappedMemoryRangeForCopy(copy_size,
                                                                              offset,
                                                                              required_size_multiple,
                                                                              this->getAllocatedByteSize(),
                                                                              this->staging_buffer.getMemory());
        this->device_manager->getDevice().flushMappedMemoryRanges({range});

        memoryUtil::unmapMemory(this->staging_buffer.getMemory(),
                                this->device_manager->getDevice());


        copyBuffer::recordCopyBuffer(this->command.buffer, this->staging_buffer, *this);

        vk::SubmitInfo si;
        si.setCommandBufferCount(1)
            .setPCommandBuffers(&this->command.buffer);

        fenceUtil::resetFence(this->command.fence, this->device_manager->getDevice());

        _wassert_result(this->graphics_queue.submit(1, &si, this->command.fence),
                        "submitting vertex buffer copy command");

        fenceUtil::awaitFence(this->command.fence, this->device_manager->getDevice());
    }

    InternallyStagedIndexBuffer::~InternallyStagedIndexBuffer() {
        this->command_manager->destroyGraphicsCommands({this->command});
    }
};
