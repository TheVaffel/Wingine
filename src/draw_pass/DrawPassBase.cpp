#include "./DrawPassBase.hpp"

namespace wg::internal {
    DrawPassBase::DrawPassBase(uint32_t num_semaphores,
                               std::shared_ptr<const DeviceManager> device_manager)
        : signal_and_wait_semaphores(num_semaphores, device_manager),
          device_manager(device_manager) { }

    SemaphoreChainPtr DrawPassBase::createAndAddOnFinishSemaphore() {
        return this->signal_and_wait_semaphores.createAndAddOnFinishSemaphore();
    }

    void DrawPassBase::setOnFinishSemaphores(const SignalSemaphoreSet& semaphores) {
        return this->signal_and_wait_semaphores.setOnFinishSemaphores(semaphores);
    }

    void DrawPassBase::setWaitSemaphores(const WaitSemaphoreSet& semaphores) {
        return this->signal_and_wait_semaphores.setWaitSemaphores(semaphores);
    }

    SignalSemaphoreSet& DrawPassBase::getSignalSemaphores() {
        return this->signal_and_wait_semaphores.getSignalSemaphores();
    }

    WaitSemaphoreSet& DrawPassBase::getWaitSemaphores() {
        return this->signal_and_wait_semaphores.getWaitSemaphores();
    }
};
