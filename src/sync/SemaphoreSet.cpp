#include "./SemaphoreSet.hpp"

#include <iostream>

namespace wg::internal {

    static std::vector<vk::Semaphore> global_empty_raw_semaphores;

    namespace {
        std::vector<std::vector<vk::Semaphore>>
        createRawSemaphoreChains(
                                 const std::vector<std::shared_ptr<ManagedSemaphoreChain>>& semaphore_chains,
                                 std::vector<uint32_t> extra_offsets = std::vector<uint32_t>()) {

            if (semaphore_chains.size() == 0) {
                return std::vector<std::vector<vk::Semaphore>>();
            }

            auto chains = std::vector<std::vector<vk::Semaphore>>(semaphore_chains[0]->getNumSemaphores());

            if (extra_offsets.size() == 0) {
                extra_offsets = std::vector<uint32_t>(semaphore_chains.size(), 0);
            }

            for (uint32_t i = 0; i < chains.size(); i++) {
                chains[i].reserve(semaphore_chains.size());

                for (uint32_t j = 0; j < semaphore_chains.size(); j++) {
                    chains[i].push_back(semaphore_chains[j]->getSemaphoreRelativeToCurrent(i + extra_offsets[j]));

                }
            }

            return chains;
        }
    };

    SemaphoreSet::SemaphoreSet(const std::initializer_list<std::shared_ptr<ManagedSemaphoreChain>>& semaphores)
        : semaphores(semaphores),
          raw_semaphores(createRawSemaphoreChains(semaphores)),
          current_semaphore_index(0)
    { }

    const std::vector<std::shared_ptr<ManagedSemaphoreChain>> SemaphoreSet::getSemaphores() const {
        return this->semaphores;
    }

    const std::vector<vk::Semaphore>& SemaphoreSet::getCurrentRawSemaphores() const {
        if (raw_semaphores.size() == 0) {
            return global_empty_raw_semaphores;
        }

        return this->raw_semaphores[current_semaphore_index];
    }

    uint32_t SemaphoreSet::getSemaphoreChainLength() const {
        if (this->getNumSemaphores() == 0) {
            return 1;
        } else {
            return this->semaphores[0]->getNumSemaphores();
        }
    }

    uint32_t SemaphoreSet::getNumSemaphores() const {
        return this->semaphores.size();
    }

    void SemaphoreSet::addSemaphoreChainAsUnsignalled(std::shared_ptr<ManagedSemaphoreChain> chain) {
        this->semaphores.push_back(chain);

        this->reinitializeRawSemaphores();
    }

    void SemaphoreSet::addSemaphoreChainAsSignalled(std::shared_ptr<ManagedSemaphoreChain> chain) {
        this->semaphores.push_back(chain);

        this->reinitializeRawSemaphoresLastOffsetOne();
    }

    void SemaphoreSet::reinitializeRawSemaphores() {
        this->raw_semaphores = createRawSemaphoreChains(this->semaphores);
        this->current_semaphore_index = 0;
    }

    void SemaphoreSet::reinitializeRawSemaphoresLastOffsetOne() {
        std::vector<uint32_t> extra_offsets(this->getNumSemaphores(), 0);
        *extra_offsets.rbegin() = 1;
        this->raw_semaphores = createRawSemaphoreChains(this->semaphores, extra_offsets);
    }

    void SemaphoreSet::swapSemaphoresFromWait() {
        for (const std::shared_ptr<ManagedSemaphoreChain>& semaphore_chain : this->semaphores) {
            semaphore_chain->swapSemaphore();
        }

        current_semaphore_index = (current_semaphore_index + 1) % this->getSemaphoreChainLength();
    }

    void SemaphoreSet::swapSemaphoresFromSignal() {
        current_semaphore_index = (current_semaphore_index + 1) % this->getSemaphoreChainLength();
    }
};
