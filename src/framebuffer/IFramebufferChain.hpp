#pragma once

#include "./IFramebuffer.hpp"

#include "../sync/ISynchronizedQueueOperation.hpp"

#include "../core/IElementChain.hpp"

namespace wg::internal {
    class IFramebufferChain : virtual ISynchronizedQueueOperation, public virtual IElementChain {
    public:

        virtual const IFramebuffer& getFramebuffer(uint32_t index) const = 0;
        IFramebuffer& getFramebuffer(uint32_t index);

        virtual const IFramebuffer& getCurrentFramebuffer() const = 0;
        IFramebuffer& getCurrentFramebuffer();

        virtual void setPresentWaitSemaphores(WaitSemaphoreSet&& semaphores) = 0;

        ~IFramebufferChain();
    };
};
