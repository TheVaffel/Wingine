#pragma once

#include <vulkan/vulkan.hpp>

#include "./queueUtils.hpp"
#include "./DeviceManager.hpp"

namespace wg::internal {

    struct QueueCollection {
        vk::Queue graphics;
        vk::Queue present;
        vk::Queue compute;
    };

    class QueueManager {
        QueueIndices queue_indices;
        QueueCollection queues;

        std::shared_ptr<const DeviceManager> device_manager;

    public:

        QueueManager(std::shared_ptr<const DeviceManager> device_manager, vk::SurfaceKHR surface);
        ~QueueManager();

        QueueManager(const QueueManager&) = delete;

        const int getGraphicsQueueIndex() const;
        const int getPresentQueueIndex() const;
        const int getComputeQueueIndex() const;

        const vk::Queue getGraphicsQueue() const;
        const vk::Queue getPresentQueue() const;
        const vk::Queue getComputeQueue() const;

        bool hasComputeQueue() const;
    };
};
