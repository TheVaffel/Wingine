#include "./StagingBuffer.hpp"

#include "./bufferUtil.hpp"
#include "./copyBuffer.hpp"
#include "../memory/memoryUtil.hpp"
#include "../sync/fenceUtil.hpp"

#include "../util/log.hpp"

#include <flawed_assert.hpp>

namespace wg::internal {
    StagingBuffer::StagingBuffer(uint32_t byte_size,
                                 std::shared_ptr<DeviceManager> device_manager,
                                 std::shared_ptr<QueueManager> queue_manager,
                                 std::shared_ptr<CommandManager> command_manager)
        : buffer(byte_size,
                 vk::BufferUsageFlagBits::eTransferSrc,
                 vk::MemoryPropertyFlagBits::eHostVisible,
                 device_manager), device_manager(device_manager), command_manager(command_manager) {
        this->graphics_queue = queue_manager->getGraphicsQueue();
        this->command = command_manager->createGraphicsCommands(1)[0];
    }

    void StagingBuffer::set(const void* src, IBuffer& dst, uint32_t byte_size, uint32_t dst_byte_offset) {
        fl_assert_le(dst_byte_offset + byte_size, dst.getByteSize());
        fl_assert_le(byte_size, this->buffer.getByteSize());

        uint32_t required_size_multiple = this->device_manager->getDeviceProperties().limits.nonCoherentAtomSize;

        vk::MappedMemoryRange range = bufferUtil::getMappedMemoryRangeForCopy(byte_size,
                                                                              0,
                                                                              required_size_multiple,
                                                                              this->buffer.getAllocatedByteSize(),
                                                                              this->buffer.getMemory());

        void* staging_dst = memoryUtil::mapMemory<void>(this->buffer.getMemory(),
                                                this->device_manager->getDevice());

        memcpy(staging_dst, src, byte_size);

        this->device_manager->getDevice().flushMappedMemoryRanges({range});

        memoryUtil::unmapMemory(this->buffer.getMemory(),
                                this->device_manager->getDevice());


        copyBuffer::recordCopyBuffer(this->command.buffer, this->buffer, dst, byte_size, 0, dst_byte_offset);

        vk::SubmitInfo si;
        si.setCommandBufferCount(1)
            .setPCommandBuffers(&this->command.buffer);

        fenceUtil::resetFence(this->command.fence, this->device_manager->getDevice());

        _wassert_result(this->graphics_queue.submit(1, &si, this->command.fence),
                        "submitting vertex buffer copy command");

        fenceUtil::awaitFence(this->command.fence, this->device_manager->getDevice());
    }

    StagingBuffer::~StagingBuffer() {
        this->command_manager->destroyGraphicsCommands({this->command});
    }
};
