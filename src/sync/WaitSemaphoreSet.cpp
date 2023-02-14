#include "./WaitSemaphoreSet.hpp"

#include <iostream>


namespace wg::internal {
    WaitSemaphoreSet::WaitSemaphoreSet(uint32_t chain_length,
                                       std::shared_ptr<const DeviceManager> device_manager)
        : SemaphoreSetBase(SemaphoreSetType::Wait, chain_length, device_manager) { }


    WaitSemaphoreSet::WaitSemaphoreSet(const std::initializer_list<SemaphoreChainPtr>& semaphores)
        : SemaphoreSetBase(SemaphoreSetType::Wait, semaphores) { }

    WaitSemaphoreSet::WaitSemaphoreSet(const std::vector<SemaphoreChainPtr>& semaphores)
        : SemaphoreSetBase(SemaphoreSetType::Wait, semaphores) { }

    void WaitSemaphoreSet::swapSemaphores() {
        this->raw_semaphores_index.incrementIndex();

        for (SemaphoreChainPtr& semaphore_chain : this->semaphore_chains) {
            semaphore_chain->swapWaitSemaphore();
        }
    }
};
