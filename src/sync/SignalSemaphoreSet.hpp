#pragma once

#include "./SemaphoreSetBase.hpp"

namespace wg::internal {
    class SignalSemaphoreSet : public SemaphoreSetBase {
    public:

        SignalSemaphoreSet(uint32_t chain_length,
                           std::shared_ptr<const DeviceManager> device_manager);

        SignalSemaphoreSet(const std::vector<SemaphoreChainPtr>& semaphores);
        SignalSemaphoreSet(const std::initializer_list<SemaphoreChainPtr>& semaphores);

        SignalSemaphoreSet(const SignalSemaphoreSet& semaphore_set) = delete;

        virtual void swapSemaphores() override;

        SemaphoreChainPtr addSignalledSemaphoreChain(const vk::Queue& queue);
    };
};
