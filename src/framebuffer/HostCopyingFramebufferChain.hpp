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
        IndexCounter inner_framebuffer_index_counter;

        std::shared_ptr<HostVisibleImageView> dst_image;
        BasicFramebufferChain<BasicFramebuffer> inner_framebuffer_chain;

        virtual SignalAndWaitSemaphores& getSemaphores() override;

    public:
        HostCopyingFramebufferChain(uint32_t num_framebuffers,
                                    std::shared_ptr<HostVisibleImageView> dst_image_view,
                                    std::shared_ptr<const QueueManager> queue_manager,
                                    std::shared_ptr<const CommandManager> command_manager,
                                    std::shared_ptr<const DeviceManager> device_manager,
                                    CompatibleRenderPassRegistry& render_pass_registry);

        virtual uint32_t getNumFramebuffers() const override;
        virtual const IFramebuffer& getFramebuffer(uint32_t index) const override;
        virtual const IFramebuffer& getCurrentFramebuffer() const override;

        virtual void setPresentWaitSemaphores(WaitSemaphoreSet&& semaphores) override;
        virtual SemaphoreChainPtr addSignalImageAcquiredSemaphore() override;
        virtual void setSignalImageAcquiredSemaphores(SignalSemaphoreSet&& semaphores) override;

        virtual void swapFramebuffer() override;
    };
};
