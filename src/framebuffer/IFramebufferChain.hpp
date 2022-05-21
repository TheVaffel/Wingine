#pragma once

#include "./IFramebuffer.hpp"

#include "../sync/ISynchronizedQueueOperation.hpp"

namespace wg::internal {
    class IFramebufferChain : virtual ISynchronizedQueueOperation {
    public:

        virtual uint32_t getNumFramebuffers() const = 0;
        virtual const IFramebuffer& getFramebuffer(uint32_t index) const = 0;
        IFramebuffer& getFramebuffer(uint32_t index);

        virtual const IFramebuffer& getCurrentFramebuffer() const = 0;
        IFramebuffer& getCurrentFramebuffer();

        virtual void swapFramebuffer() = 0;

        virtual void setPresentWaitSemaphores(WaitSemaphoreSet&& semaphores) = 0;
        virtual SemaphoreChainPtr addSignalImageAcquiredSemaphore() = 0;
        virtual void setSignalImageAcquiredSemaphores(SignalSemaphoreSet&& semaphores) = 0;

        ~IFramebufferChain();
    };
};
