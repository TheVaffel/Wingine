#include "./ManagedSemaphoreChain.hpp"

namespace wg::internal {

    namespace {
        vk::Semaphore createSemaphore(const vk::Device& device) {
            vk::SemaphoreCreateInfo ci;
            return device.createSemaphore(ci);
        }
    };

    ManagedSemaphoreChain::ManagedSemaphoreChain(uint32_t num_semaphores,
                                                 std::shared_ptr<const DeviceManager> device_manager)
        : semaphores(num_semaphores), device_manager(device_manager) {

        this->current_semaphore_index = 0;

        for (uint32_t i = 0; i < this->semaphores.size(); i++) {
            this->semaphores[i] = createSemaphore(device_manager->getDevice());
        }
    }

    void ManagedSemaphoreChain::swapSemaphore() {
        this->current_semaphore_index = (this->current_semaphore_index + 1) % this->semaphores.size();
    }

    const uint32_t ManagedSemaphoreChain::getNumSemaphores() const {
        return this->semaphores.size();
    }

    const vk::Semaphore ManagedSemaphoreChain::getSemaphoreRelativeToCurrent(uint32_t index) const {
        return this->semaphores[(this->current_semaphore_index + index) % this->semaphores.size()];
    }

    ManagedSemaphoreChain::~ManagedSemaphoreChain() {
        for (const vk::Semaphore& semaphore : this->semaphores) {
            this->device_manager->getDevice().destroy(semaphore);
        }
    }
};
