#pragma once

#include "./IFramebuffer.hpp"
#include "../semaphore.hpp"

namespace wg::internal {
    class IFramebufferChain {
    public:
        virtual uint32_t getNumFramebuffers() const = 0;
        virtual const IFramebuffer& getFramebuffer(uint32_t index) const = 0;
        virtual IFramebuffer& getFramebuffer(uint32_t index) = 0;

        virtual const IFramebuffer& getCurrentFramebuffer() const = 0;
        virtual IFramebuffer& getCurrentFramebuffer() = 0;

        virtual void swapFramebuffer() = 0;

        ~IFramebufferChain();
    };
};
