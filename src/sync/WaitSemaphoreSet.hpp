#pragma once

#include "./ManagedSemaphoreChain.hpp"

namespace wg::internal {
    class WaitSemaphoreSet {
        uint32_t chain_length;
        IndexCounter raw_semaphores_index;

        std::shared_ptr<const DeviceManager> device_manager;

        std::vector<std::shared_ptr<ManagedSemaphoreChain>> semaphore_chains;
        std::vector<std::vector<vk::Semaphore>> raw_semaphores;

    public:

        WaitSemaphoreSet(uint32_t chain_length,
                         std::shared_ptr<const DeviceManager> device_manager);
        WaitSemaphoreSet(const std::initializer_list<std::shared_ptr<ManagedSemaphoreChain>>& semaphores);

        WaitSemaphoreSet(const WaitSemaphoreSet& semaphore_set) = delete;
        WaitSemaphoreSet& operator=(const WaitSemaphoreSet& semaphore_set);

        const std::vector<vk::Semaphore>& getCurrentRawSemaphores() const;
        uint32_t getNumSemaphores() const;

        std::shared_ptr<ManagedSemaphoreChain> addSemaphoreChain();

        void swapSemaphores();

        ~WaitSemaphoreSet();
    };
};
