#pragma once

#include "./IFramebuffer.hpp"

#include "../sync/WaitSemaphoreSet.hpp"
#include "../sync/SignalSemaphoreSet.hpp"

namespace wg::internal {
    class IFramebufferChain {
    public:

        virtual uint32_t getNumFramebuffers() const = 0;
        virtual const IFramebuffer& getFramebuffer(uint32_t index) const = 0;
        virtual IFramebuffer& getFramebuffer(uint32_t index) = 0;

        virtual const IFramebuffer& getCurrentFramebuffer() const = 0;
        virtual IFramebuffer& getCurrentFramebuffer() = 0;

        virtual void swapFramebuffer() = 0;

        virtual void setPresentWaitSemaphores(const WaitSemaphoreSet& semaphores) = 0;
        virtual std::shared_ptr<ManagedSemaphoreChain> addSignalImageAcquiredSemaphore() = 0;
        virtual void setSignalImageAcquiredSemaphores(const SignalSemaphoreSet& semaphores) = 0;

        ~IFramebufferChain();
    };
};
