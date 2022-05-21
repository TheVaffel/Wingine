#include "./SignalAndWaitSemaphores.hpp"

namespace wg::internal {

    SignalAndWaitSemaphores::SignalAndWaitSemaphores(
        uint32_t chain_length,
        std::shared_ptr<const DeviceManager> device_manager)
        : signal_semaphore_set(chain_length, device_manager),
          wait_semaphore_set(chain_length, device_manager)
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

    void SignalAndWaitSemaphores::setOnFinishSemaphores(SignalSemaphoreSet&& semaphores) {
        this->signal_semaphore_set.adoptFrom(std::move(semaphores));
    }

    void SignalAndWaitSemaphores::setWaitSemaphores(WaitSemaphoreSet&& wait_semaphore_set) {
        this->wait_semaphore_set.adoptFrom(std::move(wait_semaphore_set));
    }

};
