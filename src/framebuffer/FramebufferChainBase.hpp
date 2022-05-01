#pragma once

#include "./IFramebufferChain.hpp"

#include "../sync/SynchronizedQueueOperationBase.hpp"

#include "../QueueManager.hpp"

namespace wg::internal {
    class FramebufferChainBase : public virtual IFramebufferChain, protected SynchronizedQueueOperationBase {
    protected:

        std::shared_ptr<const QueueManager> queue_manager;
        std::shared_ptr<const DeviceManager> device_manager;

        FramebufferChainBase(uint32_t num_semaphores,
                             std::shared_ptr<const QueueManager> queue_manager,
                             std::shared_ptr<const DeviceManager> device_manager);

    public:
        virtual void setPresentWaitSemaphores(const WaitSemaphoreSet& semaphores);
        virtual SemaphoreChainPtr addSignalImageAcquiredSemaphore();
        virtual void setSignalImageAcquiredSemaphores(const SignalSemaphoreSet& semaphores);
    };
};
