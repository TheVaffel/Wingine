#include "./StagingBufferManager.hpp"

#include "./BasicBuffer.hpp"

#include "../memory/memoryUtil.hpp"
#include "../sync/fenceUtil.hpp"

#include "./copyBuffer.hpp"

#include "../util/log.hpp"

#include <flawed_assert.hpp>

namespace wg::internal {

    StagingBufferManager::StagingBufferManager(std::shared_ptr<const CommandManager> command_manager,
                                               std::shared_ptr<const QueueManager> queue_manager,
                                               std::shared_ptr<const DeviceManager> device_manager)
        : device_manager(device_manager), queue_manager(queue_manager) {
        this->buffer = BasicBuffer::createHostVisibleBuffer(StagingBufferManager::BUFFER_MAX_SIZE,
                                                            device_manager);

        this->command = command_manager->createGraphicsCommands(1)[0];
    }

    void StagingBufferManager::copySync(const void *src, const IBuffer& dst, uint32_t byte_count) {
        fenceUtil::awaitAndResetFence(this->command.fence, this->device_manager->getDevice());

        fl_assert_le(byte_count, StagingBufferManager::BUFFER_MAX_SIZE);

        void * temp_buf = memoryUtil::mapMemory<void>(this->buffer->getMemory(),
                                                      this->device_manager->getDevice());

        memcpy(temp_buf, src, byte_count);

        memoryUtil::unmapMemory(this->buffer->getMemory(),
                                this->device_manager->getDevice());

        copyBuffer::recordCopyBuffer(this->command.buffer,
                                     *this->buffer,
                                     dst);

        vk::SubmitInfo submit_info;
        submit_info.setCommandBufferCount(1)
            .setPCommandBuffers(&this->command.buffer)
            .setWaitSemaphoreCount(0)
            .setSignalSemaphoreCount(0);

        _wassert_result(this->queue_manager->getGraphicsQueue()
                        .submit(1, &submit_info, this->command.fence),
                        "command submission from StagingBufferManager");
    }

    StagingBufferManager::~StagingBufferManager() {
        this->device_manager->getDevice().destroyFence(this->command.fence);
    }
};
