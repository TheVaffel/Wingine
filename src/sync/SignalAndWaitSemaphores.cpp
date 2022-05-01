#include "./SignalAndWaitSemaphores.hpp"

namespace wg::internal {

    SignalAndWaitSemaphores::SignalAndWaitSemaphores(
        uint32_t num_chains,
        std::shared_ptr<const DeviceManager> device_manager)
        : signal_semaphore_set(num_chains, device_manager),
          wait_semaphore_set(num_chains, device_manager)
        { }

    SignalSemaphoreSet& SignalAndWaitSemaphores::getSignalSemaphores() {
        return this->signal_semaphore_set;
    }

    WaitSemaphoreSet& SignalAndWaitSemaphores::getWaitSemaphores() {
        return this->wait_semaphore_set;
    }

    const SignalSemaphoreSet& SignalAndWaitSemaphores::getSignalSemaphores() const {
        return this->signal_semaphore_set;
    }

    const WaitSemaphoreSet& SignalAndWaitSemaphores::getWaitSemaphores() const {
        return this->wait_semaphore_set;
    }

    SemaphoreChainPtr SignalAndWaitSemaphores::createOnFinishSemaphore() {
        return this->signal_semaphore_set.addSemaphoreChain();
    }

    void SignalAndWaitSemaphores::setOnFinishSemaphores(const SignalSemaphoreSet& semaphores) {
        this->signal_semaphore_set = semaphores;
    }

    void SignalAndWaitSemaphores::setWaitSemaphores(const WaitSemaphoreSet& semaphore_semaphore_set) {
        this->wait_semaphore_set = semaphore_semaphore_set;
    }

};
