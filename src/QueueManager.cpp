#include "./QueueManager.hpp"

#include "./log.hpp"

namespace wg::internal {

    namespace {

        void sanityCheckIndicesForGraphics(const QueueIndices& indices) {
            if (indices.graphics == -1) {
                _wlog_error("Chosen graphics device does not support graphics");
            }
        }

        void sanityCheckIndicesForPresent(const QueueIndices& indices) {
            if (indices.present == -1) {
                _wlog_error("Chosen graphics device does not support present");
            }
        }

        void sanityCheckIndicesForCompute(const QueueIndices& indices) {
            if(indices.compute == -1) {
                _wlog_warn("Chosen graphics device does not support compute kernels");
            }
        }

        void sanityCheckIndices(const QueueIndices& indices) {
            sanityCheckIndicesForGraphics(indices);
            sanityCheckIndicesForPresent(indices);
            sanityCheckIndicesForCompute(indices);
        }

        void sanityCheckIndicesWithoutSurface(const QueueIndices& indices) {
            sanityCheckIndicesForGraphics(indices);
            sanityCheckIndicesForCompute(indices);
        }

        QueueCollection getQueues(const DeviceManager& device_manager,
                                  const QueueIndices& indices) {

            vk::Queue graphics_queue = device_manager.getDevice().getQueue(indices.graphics, 0);

            vk::Queue present_queue;

            if (indices.present != -1) {
                // If graphics and present queue indices are equal, make queues equal
                if(indices.graphics == indices.present) {
                    present_queue = graphics_queue;
                } else {
                    present_queue =
                        device_manager.getDevice().getQueue(indices.present, 0);
                }
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
                               const VulkanInstanceManager& instance_manager)
        : device_manager(device_manager) {

        if (instance_manager.hasSurface()) {
            this->queue_indices = getQueueIndicesForDevice(device_manager->getPhysicalDevice(),
                                                           instance_manager.getSurface());

            sanityCheckIndices(this->queue_indices);
        } else {
            this->queue_indices = getQueueIndicesForDeviceWithoutSurface(device_manager->getPhysicalDevice());
            sanityCheckIndicesWithoutSurface(this->queue_indices);
        }

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
        if (!this->hasPresentQueue()) {
            throw std::runtime_error("[QueueManager] Does not have a present queue");
        }
        return this->queues.present;
    }

    const vk::Queue QueueManager::getComputeQueue() const {
        return this->queues.compute;
    }

    bool QueueManager::hasPresentQueue() const {
        return this->queue_indices.present != -1;
    }

    bool QueueManager::hasComputeQueue() const {
        return this->queue_indices.compute != -1;
    }
};
