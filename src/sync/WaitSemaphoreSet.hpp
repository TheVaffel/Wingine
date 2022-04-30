#pragma once

#include "./SemaphoreSetBase.hpp"

namespace wg::internal {
    class WaitSemaphoreSet : public SemaphoreSetBase {
    public:

        WaitSemaphoreSet(uint32_t chain_length,
                         std::shared_ptr<const DeviceManager> device_manager);
        WaitSemaphoreSet(const std::initializer_list<SemaphoreChainPtr>& semaphores);

        WaitSemaphoreSet(const WaitSemaphoreSet& semaphore_set) = delete;

        virtual void swapSemaphores() override;
    };
};
