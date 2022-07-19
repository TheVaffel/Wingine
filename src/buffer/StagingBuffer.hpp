#pragma once

#include "./BasicBuffer.hpp"

#include "../CommandManager.hpp"

namespace wg::internal {
    class StagingBuffer {
        BasicBuffer buffer;

        Command command;
        vk::Queue graphics_queue;

        std::shared_ptr<DeviceManager> device_manager;
        std::shared_ptr<CommandManager> command_manager;

    public:
        StagingBuffer(uint32_t byte_size,
                      std::shared_ptr<DeviceManager> device_manager,
                      std::shared_ptr<QueueManager> queue_manager,
                      std::shared_ptr<CommandManager> command_manager);

        void set(const void* src, IBuffer& dst, uint32_t byte_size, uint32_t dst_byte_offset);

        ~StagingBuffer();
    };
};
