#pragma once

#include "./WaitSemaphoreSet.hpp"
#include "./SignalSemaphoreSet.hpp"

namespace wg::internal {
    class SignalAndWaitSemaphores {
        SignalSemaphoreSet signal_semaphore_set;
        WaitSemaphoreSet wait_semaphore_set;
    public:

        SignalAndWaitSemaphores(uint32_t num_chains,
                                std::shared_ptr<const DeviceManager> device_manager);

        const SignalSemaphoreSet& getSignalSemaphores() const;
        const WaitSemaphoreSet& getWaitSemaphores() const;

        SignalSemaphoreSet& getSignalSemaphores();
        WaitSemaphoreSet& getWaitSemaphores();

        void setWaitSemaphores(const WaitSemaphoreSet& semaphores);
        void setOnFinishSemaphores(const SignalSemaphoreSet& semaphores);

        [[nodiscard]]
        SemaphoreChainPtr createOnFinishSemaphore();
    };
};
