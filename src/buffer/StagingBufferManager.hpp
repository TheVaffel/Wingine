#pragma once

#include "./IBuffer.hpp"
#include "../core/DeviceManager.hpp"
#include "../core/CommandManager.hpp"

namespace wg::internal {
    class StagingBufferManager {

        std::unique_ptr<IBuffer> buffer;
        std::shared_ptr<const DeviceManager> device_manager;
        std::shared_ptr<const QueueManager> queue_manager;

        Command command;

    public:
        static constexpr uint32_t BUFFER_MAX_SIZE = 10 * 1024 * 1024;

        StagingBufferManager(std::shared_ptr<const CommandManager> command_manager,
                             std::shared_ptr<const QueueManager> queue_manager,
                             std::shared_ptr<const DeviceManager> device_manager);

        void copySync(const void *src, const IBuffer& dst, uint32_t byte_count);

        std::shared_ptr<StagingBufferCopyOperation> createCopyOperation(uint32_t byteSize);

        ~StagingBufferManager();
    };
};
