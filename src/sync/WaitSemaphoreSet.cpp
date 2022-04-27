#include "./WaitSemaphoreSet.hpp"

#include <iostream>

namespace wg::internal {

    namespace {
        uint32_t getChainLengthFromSemaphores(
            const std::initializer_list<std::shared_ptr<ManagedSemaphoreChain>>& semaphores) {
            if (semaphores.size() == 0) {
                throw std::runtime_error(
                    "[WaitSemaphoreSet] Cannot find chain length from zero-length semaphore list");
            }

            return std::begin(semaphores)[0]->getNumSemaphores();
        }

        std::shared_ptr<const DeviceManager>  getDeviceManagerFromSemaphores(
            const std::initializer_list<std::shared_ptr<ManagedSemaphoreChain>>& semaphores) {
            if (semaphores.size() == 0) {
                throw std::runtime_error(
                    "[WaitSemaphoreSet] Cannot find device manager from zero-length semaphore list");
            }

            return std::begin(semaphores)[0]->getDeviceManager();
        }

        std::vector<std::vector<vk::Semaphore>>
        createRawWaitSemaphoreChains(const std::vector<std::shared_ptr<ManagedSemaphoreChain>>& semaphore_chains) {
            if (semaphore_chains.size() == 0) {
                return std::vector<std::vector<vk::Semaphore>>();
            }

            std::vector<std::vector<vk::Semaphore>> chains(semaphore_chains[0]->getNumSemaphores());

            for (uint32_t i = 0; i < chains.size(); i++) {
                chains[i].reserve(semaphore_chains.size());

                for (uint32_t j = 0; j < semaphore_chains.size(); j++) {
                    chains[i].push_back(semaphore_chains[j]->getSemaphoreRelativeToCurrentWait(i));
                }
            }

            return chains;
        }
    };

    WaitSemaphoreSet::WaitSemaphoreSet(uint32_t chain_length,
                                       std::shared_ptr<const DeviceManager> device_manager)
        : chain_length(chain_length),
          raw_semaphores_index(chain_length),
          device_manager(device_manager) { }


    WaitSemaphoreSet::WaitSemaphoreSet(const std::initializer_list<
                                       std::shared_ptr<ManagedSemaphoreChain>>& semaphores)
        : chain_length(getChainLengthFromSemaphores(semaphores)),
          raw_semaphores_index(chain_length),
          device_manager(getDeviceManagerFromSemaphores(semaphores)),
          semaphore_chains(semaphores),
          raw_semaphores(createRawWaitSemaphoreChains(semaphore_chains)) {

        for (std::shared_ptr<ManagedSemaphoreChain>& semaphore_chain : this->semaphore_chains) {
            semaphore_chain->registerWaitSet();
        }
    }

    WaitSemaphoreSet& WaitSemaphoreSet::operator=(const WaitSemaphoreSet& semaphore_set) {
        for (std::shared_ptr<ManagedSemaphoreChain> semaphore_chain : this->semaphore_chains) {
            semaphore_chain->unregisterWaitSet();
        }

        this->semaphore_chains = semaphore_set.semaphore_chains;
        this->raw_semaphores = createRawWaitSemaphoreChains(this->semaphore_chains);

        for (std::shared_ptr<ManagedSemaphoreChain> semaphore_chain : this->semaphore_chains) {
            semaphore_chain->registerWaitSet();
        }

        return *this;
    }


    const std::vector<vk::Semaphore>& WaitSemaphoreSet::getCurrentRawSemaphores() const {
        return this->raw_semaphores[this->raw_semaphores_index.getCurrentIndex()];
    }

    uint32_t WaitSemaphoreSet::getNumSemaphores() const {
        return this->semaphore_chains.size();
    }

    std::shared_ptr<ManagedSemaphoreChain> WaitSemaphoreSet::addSemaphoreChain() {
        std::shared_ptr<ManagedSemaphoreChain> semaphore_chain =
            std::make_shared<ManagedSemaphoreChain>(this->chain_length,
                                                    this->device_manager);

        semaphore_chain->registerWaitSet();
        this->semaphore_chains.push_back(semaphore_chain);
        this->raw_semaphores = createRawWaitSemaphoreChains(this->semaphore_chains);

        return semaphore_chain;
    }

    void WaitSemaphoreSet::swapSemaphores() {
        this->raw_semaphores_index.incrementIndex();

        for (std::shared_ptr<ManagedSemaphoreChain>& semaphore_chain : this->semaphore_chains) {
            semaphore_chain->swapWaitSemaphore();
        }
    }

    WaitSemaphoreSet::~WaitSemaphoreSet() {
        for (std::shared_ptr<ManagedSemaphoreChain>& semaphore_chain : this->semaphore_chains) {
            semaphore_chain->unregisterWaitSet();
        }
    }
};
