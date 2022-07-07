#pragma once

#include "./InternallyStagedVertexBuffer.hpp"
#include "../memory/memoryUtil.hpp"
#include "./copyBuffer.hpp"
#include "../sync/fenceUtil.hpp"

#include "../util/log.hpp"

namespace wg::internal {
    template<typename T>
    InternallyStagedVertexBuffer<T>::InternallyStagedVertexBuffer(uint32_t num_elements,
                                                                  std::shared_ptr<DeviceManager> device_manager,
                                                                  std::shared_ptr<QueueManager> queue_manager,
                                                                  std::shared_ptr<CommandManager> command_manager)
        : BasicBuffer(num_elements * sizeof(T),
                      vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                      vk::MemoryPropertyFlagBits::eDeviceLocal,
                      device_manager),
          staging_buffer(num_elements * sizeof(T),
                         vk::BufferUsageFlagBits::eTransferSrc,
                         vk::MemoryPropertyFlagBits::eHostVisible,
                         device_manager),
          device_manager(device_manager),
          command_manager(command_manager) {
        this->command = this->command_manager->createGraphicsCommands(1)[0];
        this->graphics_queue = queue_manager->getGraphicsQueue();
    }


    template<typename T>
    void InternallyStagedVertexBuffer<T>::set(const T* data, uint32_t first_element, uint32_t element_count) {
        vk::MappedMemoryRange range;
        range.setMemory(this->staging_buffer.getMemory())
            .setOffset(first_element * sizeof(T))
            .setSize(element_count * sizeof(T));

        T* dst = memoryUtil::mapMemory<T>(this->staging_buffer.getMemory(),
                                          this->device_manager->getDevice());

        memcpy(dst, data, sizeof(T) * element_count);

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

    template<typename T>
    InternallyStagedVertexBuffer<T>::~InternallyStagedVertexBuffer() {
        this->command_manager->destroyGraphicsCommands({this->command});
    }
};
