#include "./QueueManager.hpp"

#include "./log.hpp"

namespace wg::internal {

    namespace {
        void sanityCheckIndices(const QueueIndices& indices) {

            if (indices.graphics == -1) {
                _wlog_error("Chosen graphics device does not support graphics");
            }

            if (indices.present == -1) {
                _wlog_error("Chosen graphics device does not support present");
            }

            if(indices.compute == -1) {
                _wlog_warn("Chosen graphics device does not support compute kernels");
            }
        }

        QueueCollection getQueues(const DeviceManager& device_manager,
                                  const QueueIndices& indices) {

            vk::Queue graphics_queue = device_manager.getDevice().getQueue(indices.graphics, 0);

            vk::Queue present_queue;

            // If graphics and present queue indices are equal, make queues equal
            if(indices.graphics == indices.present) {
                present_queue = graphics_queue;
            } else {
                present_queue =
                    device_manager.getDevice().getQueue(indices.present, 0);
            }

            vk::Queue compute_queue = nullptr;

            if(indices.compute != -1) {
                compute_queue =
                    device_manager.getDevice().getQueue(indices.compute, 0);
            }

            return {
                .graphics = graphics_queue,
                .present = present_queue,
                .compute = compute_queue
            };
        }
    };

    QueueManager::QueueManager(std::shared_ptr<const DeviceManager> device_manager,
                               vk::SurfaceKHR surface)
        : device_manager(device_manager) {

        this->queue_indices = getQueueIndicesForDevice(device_manager->getPhysicalDevice(), surface);

        sanityCheckIndices(this->queue_indices);

        this->queues = getQueues(*device_manager, this->queue_indices);
    }

    QueueManager::~QueueManager() { }

    const int QueueManager::getGraphicsQueueIndex() const {
        return this->queue_indices.graphics;
    }

    const int QueueManager::getPresentQueueIndex() const {
        return this->queue_indices.present;
    }

    const int QueueManager::getComputeQueueIndex() const {
        return this->queue_indices.compute;
    };

    const vk::Queue QueueManager::getGraphicsQueue() const {
        return this->queues.graphics;
    }

    const vk::Queue QueueManager::getPresentQueue() const {
        return this->queues.present;
    }

    const vk::Queue QueueManager::getComputeQueue() const {
        return this->queues.compute;
    }

    bool QueueManager::hasComputeQueue() const {
        return this->queue_indices.compute != -1;
    }
};
