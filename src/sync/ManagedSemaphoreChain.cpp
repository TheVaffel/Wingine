#include "./ManagedSemaphoreChain.hpp"

#include <flawed_assert.hpp>

namespace wg::internal {

    namespace {
        vk::Semaphore createSemaphore(const vk::Device& device) {
            vk::SemaphoreCreateInfo ci;
            return device.createSemaphore(ci);
        }
    };

    ManagedSemaphoreChain::ManagedSemaphoreChain(uint32_t num_semaphores,
                                                 std::shared_ptr<const DeviceManager> device_manager)
        : signal_index(num_semaphores),
          wait_index(num_semaphores),
          signal_set_count(0),
          wait_set_count(0),
          semaphores(num_semaphores),
          device_manager(device_manager) {

        for (uint32_t i = 0; i < this->semaphores.size(); i++) {
            this->semaphores[i] = createSemaphore(device_manager->getDevice());
        }
    }

    void ManagedSemaphoreChain::swapSignalSemaphore() {
        if (this->signal_index.getCurrentIndex() != this->wait_index.getCurrentIndex()) {
            throw std::runtime_error("Wait/signal for semaphore chain not in sync,"
                                     "did you create semaphore after starting rendering within a frame?");
        }
        fl_assert_eq(this->signal_set_count, 1);
        fl_assert_eq(this->wait_set_count, 1);
        this->signal_index.incrementIndex();
    }

    void ManagedSemaphoreChain::markAsSignalled() {
        this->signal_index.incrementIndex();
    }

    void ManagedSemaphoreChain::swapWaitSemaphore() {
        fl_assert_eq(this->signal_set_count, 1);
        fl_assert_eq(this->wait_set_count, 1);
        this->wait_index.incrementIndex();
        if (this->signal_index.getCurrentIndex() != this->wait_index.getCurrentIndex()) {
            throw std::runtime_error("Wait/signal for semaphore chain not in sync, "
                                     "did you create semaphore after starting rendering within a frame?");
        }
    }

    void ManagedSemaphoreChain::registerSignalSet() {
        this->signal_set_count++;
    }

    void ManagedSemaphoreChain::registerWaitSet() {
        this->wait_set_count++;
    }

    void ManagedSemaphoreChain::unregisterSignalSet() {
        fl_assert_ge(this->signal_set_count, 1);

        this->signal_set_count--;
    }

    void ManagedSemaphoreChain::unregisterWaitSet() {
        fl_assert_ge(this->wait_set_count, 1);

        this->wait_set_count--;
    }

    bool ManagedSemaphoreChain::isBetweenSignalAndAwait() const {
        return this->signal_index.getCurrentIndex() != this->wait_index.getCurrentIndex();
    }

    const uint32_t ManagedSemaphoreChain::getNumSemaphores() const {
        return this->semaphores.size();
    }

    const vk::Semaphore ManagedSemaphoreChain::getSemaphoreRelativeToCurrentSignal(uint32_t index) const {
        return this->semaphores[this->signal_index.getRelativeIndex(index)];
    }

    const vk::Semaphore ManagedSemaphoreChain::getSemaphoreRelativeToCurrentWait(uint32_t index) const {
        return this->semaphores[this->wait_index.getRelativeIndex(index)];
    }

    std::shared_ptr<const DeviceManager> ManagedSemaphoreChain::getDeviceManager() const {
        return this->device_manager;
    }

    ManagedSemaphoreChain::~ManagedSemaphoreChain() {
        for (const vk::Semaphore& semaphore : this->semaphores) {
            this->device_manager->getDevice().destroy(semaphore);
        }
    }
};
