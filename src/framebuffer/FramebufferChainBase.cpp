#include "./FramebufferChainBase.hpp"

namespace wg::internal {
    FramebufferChainBase::FramebufferChainBase(uint32_t num_semaphores,
                                               std::shared_ptr<const QueueManager> queue_manager,
                                               std::shared_ptr<const DeviceManager> device_manager)
        : SynchronizedQueueOperationBase(num_semaphores, device_manager),
          queue_manager(queue_manager),
          device_manager(device_manager) { }

    void FramebufferChainBase::setPresentWaitSemaphores(const WaitSemaphoreSet& semaphores) {
        this->getWaitSemaphores() = semaphores;
    }

    SemaphoreChainPtr FramebufferChainBase::addSignalImageAcquiredSemaphore() {
        return this->getSignalSemaphores().addSignalledSemaphoreChain(this->queue_manager->getGraphicsQueue());
    }


    void FramebufferChainBase::setSignalImageAcquiredSemaphores(const SignalSemaphoreSet& semaphores) {
        this->getSignalSemaphores() = semaphores;
    }
};
