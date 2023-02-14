#include "./SignalSemaphoreSet.hpp"

#include "./semaphoreUtil.hpp"

namespace wg::internal {

    namespace {
        SemaphoreChainPtr createAndSignalSemaphore(uint32_t chain_length,
                                                   const vk::Queue& queue,
                                                   std::shared_ptr<const DeviceManager> device_manager) {
            std::shared_ptr<ManagedSemaphoreChain> semaphore_chain =
                std::make_shared<ManagedSemaphoreChain>(chain_length,
                                                        device_manager);

            semaphoreUtil::signalSemaphore(semaphore_chain->getSemaphoreRelativeToCurrentSignal(0),
                                           queue);
            semaphore_chain->markAsSignalled();

            return semaphore_chain;
        }
    };

    SignalSemaphoreSet::SignalSemaphoreSet(uint32_t chain_length,
                                           std::shared_ptr<const DeviceManager> device_manager)
        : SemaphoreSetBase(SemaphoreSetType::Signal, chain_length, device_manager) { }

    SignalSemaphoreSet::SignalSemaphoreSet(const std::vector<SemaphoreChainPtr>& semaphores)
        : SemaphoreSetBase(SemaphoreSetType::Signal, semaphores) { }

    SignalSemaphoreSet::SignalSemaphoreSet(const std::initializer_list<SemaphoreChainPtr>& semaphores)
        : SemaphoreSetBase(SemaphoreSetType::Signal, semaphores) { }

    SemaphoreChainPtr SignalSemaphoreSet::addSignalledSemaphoreChain(const vk::Queue& queue) {

        SemaphoreChainPtr semaphore_chain =
            createAndSignalSemaphore(this->chain_length, queue, this->device_manager);

        semaphore_chain->registerSignalSet();

        this->semaphore_chains.push_back(semaphore_chain);
        this->initRawSemaphores();

        return semaphore_chain;
    }

    void SignalSemaphoreSet::swapSemaphores() {
        this->raw_semaphores_index.incrementIndex();

        for (SemaphoreChainPtr& semaphore_chain : this->semaphore_chains) {
            semaphore_chain->swapSignalSemaphore();
        }
    }
};
