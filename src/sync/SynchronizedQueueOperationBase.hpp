#pragma once

#include "./ISynchronizedQueueOperation.hpp"

namespace wg::internal {
    class SynchronizedQueueOperationBase : public virtual ISynchronizedQueueOperation {

        SignalAndWaitSemaphores semaphores;

    protected:
        SynchronizedQueueOperationBase(uint32_t chain_length,
                                       std::shared_ptr<const DeviceManager> device_manager);

        const SignalSemaphoreSet& getSignalSemaphores() const;
        const WaitSemaphoreSet& getWaitSemaphores() const;

        SignalSemaphoreSet& getSignalSemaphores();
        WaitSemaphoreSet& getWaitSemaphores();

        ~SynchronizedQueueOperationBase() = default;
    public:
        virtual SignalAndWaitSemaphores& getSemaphores() override;
    };
};
