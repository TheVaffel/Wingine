#pragma once

#include "./ManagedSemaphoreChain.hpp"

namespace wg::internal {
    class SemaphoreSet {
        std::vector<std::shared_ptr<ManagedSemaphoreChain>> semaphores;
        std::vector<std::vector<vk::Semaphore>> raw_semaphores;

        uint32_t current_semaphore_index;

        uint32_t getSemaphoreChainLength() const;

        void reinitializeRawSemaphores();

        /*
         * When adding a signalled semaphore while the current ones have already
         * been signalled, we must add the raw semaphores of the new one with an
         * offset, so that on the next signal operation, this next one is the one
         */
        void reinitializeRawSemaphoresLastOffsetOne();

    public:
        SemaphoreSet(const std::initializer_list<std::shared_ptr<ManagedSemaphoreChain>>& semaphores);

        const std::vector<std::shared_ptr<ManagedSemaphoreChain>> getSemaphores() const;
        // const std::vector<vk::Semaphore>& getRawSemaphores(uint32_t index) const;
        const std::vector<vk::Semaphore>& getCurrentRawSemaphores() const;
        uint32_t getNumSemaphores() const;

        void addSemaphoreChainAsUnsignalled(std::shared_ptr<ManagedSemaphoreChain> chain);
        void addSemaphoreChainAsSignalled(std::shared_ptr<ManagedSemaphoreChain> chain);

        /*
         * The underlying semaphore chains are only swapped on swapSemaphoresFromWait,
         * while the semaphore index of this is incremented in both
         */
        void swapSemaphoresFromWait();
        void swapSemaphoresFromSignal();
    };
};
