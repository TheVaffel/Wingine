#pragma once

#include "./ManagedSemaphoreChain.hpp"

namespace wg::internal {
    class SignalSemaphoreSet {
        uint32_t chain_length;
        IndexCounter raw_semaphores_index;

        std::shared_ptr<const DeviceManager> device_manager;

        std::vector<std::shared_ptr<ManagedSemaphoreChain>> semaphore_chains;
        std::vector<std::vector<vk::Semaphore>> raw_semaphores;

    public:

        SignalSemaphoreSet(uint32_t chain_length,
                         std::shared_ptr<const DeviceManager> device_manager);
        SignalSemaphoreSet(const std::initializer_list<std::shared_ptr<ManagedSemaphoreChain>>& semaphores);

        SignalSemaphoreSet(const SignalSemaphoreSet& semaphore_set) = delete;
        SignalSemaphoreSet& operator=(const SignalSemaphoreSet& semaphore_set);

        const std::vector<vk::Semaphore>& getCurrentRawSemaphores() const;
        uint32_t getNumSemaphores() const;

        std::shared_ptr<ManagedSemaphoreChain> addSemaphoreChain();
        std::shared_ptr<ManagedSemaphoreChain> addSignalledSemaphoreChain(const vk::Queue& queue);

        void swapSemaphores();

        ~SignalSemaphoreSet();
    };
};
