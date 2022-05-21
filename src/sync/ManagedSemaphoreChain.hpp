#pragma once

#include "../DeviceManager.hpp"
#include "../util/IndexCounter.hpp"

#include <vulkan/vulkan.hpp>

#include <vector>
#include <memory>

namespace wg::internal {
    class ManagedSemaphoreChain {

        IndexCounter signal_index;
        IndexCounter wait_index;

        // Sanity check that this is associated with one signal set and one wait set
        bool has_signal_set;
        bool has_wait_set;

        std::vector<vk::Semaphore> semaphores;
        std::shared_ptr<const DeviceManager> device_manager;

    public:

        ManagedSemaphoreChain(uint32_t num_semaphores,
                              std::shared_ptr<const DeviceManager> device_manager);

        void swapSignalSemaphore();
        void swapWaitSemaphore();

        // When initialized as signalled, call this
        void markAsSignalled();

        void registerSignalSet();
        void registerWaitSet();

        void unregisterSignalSet();
        void unregisterWaitSet();

        bool isBetweenSignalAndAwait() const;

        const uint32_t getNumSemaphores() const;
        const vk::Semaphore getSemaphoreRelativeToCurrentSignal(uint32_t index) const;
        const vk::Semaphore getSemaphoreRelativeToCurrentWait(uint32_t index) const;

        // Utility function for use by SemaphoreSets
        std::shared_ptr<const DeviceManager> getDeviceManager() const;

        ManagedSemaphoreChain(const ManagedSemaphoreChain&) = delete;

        ~ManagedSemaphoreChain();
    };

    using SemaphoreChainPtr = std::shared_ptr<ManagedSemaphoreChain>;
};
