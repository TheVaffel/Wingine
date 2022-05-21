#include "./SynchronizedQueueOperationBase.hpp"

namespace wg::internal {
    SynchronizedQueueOperationBase::SynchronizedQueueOperationBase(uint32_t chain_length,
                                                                   std::shared_ptr<const DeviceManager> device_manager)
        : semaphores(chain_length, device_manager) { }

    SignalAndWaitSemaphores& SynchronizedQueueOperationBase::getSemaphores() {
        return this->semaphores;
    }

    WaitSemaphoreSet& SynchronizedQueueOperationBase::getWaitSemaphores() {
        return this->semaphores.getWaitSemaphores();
    }

    SignalSemaphoreSet& SynchronizedQueueOperationBase::getSignalSemaphores() {
        return this->semaphores.getSignalSemaphores();
    }

    const WaitSemaphoreSet& SynchronizedQueueOperationBase::getWaitSemaphores() const {
        return this->semaphores.getWaitSemaphores();
    }

    const SignalSemaphoreSet& SynchronizedQueueOperationBase::getSignalSemaphores() const {
        return this->semaphores.getSignalSemaphores();
    }
}
