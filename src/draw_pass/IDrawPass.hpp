#pragma once

#include "../framebuffer/IFramebufferChain.hpp"

#include "../resource.hpp"
#include "../buffer.hpp"

#include <memory>

#include "../sync/ManagedSemaphoreChain.hpp"
#include "../sync/SignalSemaphoreSet.hpp"
#include "../sync/WaitSemaphoreSet.hpp"

namespace wg::internal {
    class IDrawPass {
    public:

        virtual void startRecording(std::shared_ptr<IFramebufferChain> framebufferChain) = 0;
        virtual void recordDraw(const std::vector<const Buffer*>& buffers, const IndexBuffer* ind_buf,
                        const std::vector<ResourceSet*>& sets, uint32_t instanceCount = 1) = 0;
        virtual void endRecording() = 0;

        [[nodiscard]]
        virtual std::shared_ptr<ManagedSemaphoreChain> createAndAddOnFinishSemaphore() = 0;

        virtual void resetOnFinishSemaphores(const SignalSemaphoreSet& semaphores) = 0;

        virtual void setWaitSemaphores(const WaitSemaphoreSet& semaphoreSet) = 0;

        virtual void render() = 0;

        virtual ~IDrawPass();
    };
};
