#pragma once

#include "./IFramebufferChain.hpp"

#include "../image/HostVisibleImageView.hpp"
#include "../image/ImageToBufferCopier.hpp"

#include "./BasicFramebufferChain.hpp"
#include "./BasicFramebuffer.hpp"

#include "../util/IndexCounter.hpp"

namespace wg::internal {

    class HostCopyingFramebufferChain : public IFramebufferChain {

        std::shared_ptr<const DeviceManager> device_manager;
        std::shared_ptr<const QueueManager> queue_manager;

        std::unique_ptr<ImageToBufferCopier> image_copier;

        std::shared_ptr<HostVisibleImageView> dst_image;
        BasicFramebufferChain<BasicFramebuffer> inner_framebuffer_chain;

        virtual SignalAndWaitSemaphores& getSemaphores() final;

    public:
        HostCopyingFramebufferChain(uint32_t num_framebuffers,
                                    std::shared_ptr<HostVisibleImageView> dst_image_view,
                                    std::shared_ptr<const QueueManager> queue_manager,
                                    std::shared_ptr<const CommandManager> command_manager,
                                    std::shared_ptr<const DeviceManager> device_manager,
                                    CompatibleRenderPassRegistry& render_pass_registry);

        virtual const IFramebuffer& getFramebuffer(uint32_t index) const final;
        virtual const IFramebuffer& getCurrentFramebuffer() const final;

        virtual void swapToNextElement() final;
        virtual uint32_t getCurrentElementIndex() const final;
        virtual uint32_t getElementChainLength() const final;

        virtual void setPresentWaitSemaphores(WaitSemaphoreSet&& semaphores) final;
        virtual SemaphoreChainPtr addSignalImageAcquiredSemaphore() final;
        virtual void setSignalImageAcquiredSemaphores(SignalSemaphoreSet&& semaphores) final;
    };
};
