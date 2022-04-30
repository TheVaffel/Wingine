#pragma once

#include "./IDrawPass.hpp"
#include "../sync/SignalAndWaitSemaphores.hpp"

namespace wg::internal {
    class DrawPassBase : public IDrawPass {

    protected:
        SignalAndWaitSemaphores signal_and_wait_semaphores;

        std::shared_ptr<const DeviceManager> device_manager;

        DrawPassBase(uint32_t num_semaphores,
                     std::shared_ptr<const DeviceManager> device_manager);

        SignalSemaphoreSet& getSignalSemaphores();
        WaitSemaphoreSet& getWaitSemaphores();
    public:

        [[nodiscard]]
        virtual SemaphoreChainPtr createAndAddOnFinishSemaphore();
        virtual void setOnFinishSemaphores(const SignalSemaphoreSet& semaphores);
        virtual void setWaitSemaphores(const WaitSemaphoreSet& semaphoreSet);
    };
};
