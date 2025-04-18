#include "./HostCopyingFramebufferChain.hpp"

namespace wg::internal {

    HostCopyingFramebufferChain::HostCopyingFramebufferChain(uint32_t num_framebuffers,
                                                             std::shared_ptr<HostVisibleImageView> dst_image,
                                                             std::shared_ptr<const QueueManager> queue_manager,
                                                             std::shared_ptr<const CommandManager> command_manager,
                                                             std::shared_ptr<const DeviceManager> device_manager,
                                                             CompatibleRenderPassRegistry& render_pass_registry)
    : device_manager(device_manager),
      queue_manager(queue_manager),
      image_copier(std::make_unique<ImageToBufferCopier>(num_framebuffers,
                                                         queue_manager->getGraphicsQueue(),
                                                         command_manager,
                                                         device_manager)),
      dst_image(dst_image),
      inner_framebuffer_chain(num_framebuffers,
                              device_manager,
                              queue_manager,
                              dst_image->getDimensions(),
                              BasicFramebufferSetup(),
                              device_manager,
                              render_pass_registry)
    {

        std::vector<IImage*> images;
        std::vector<IBuffer*> buffers;
        for (uint32_t i = 0; i < num_framebuffers; i++) {
            images.push_back(&this->inner_framebuffer_chain.IFramebufferChain::getFramebuffer(i).getColorImage());
            buffers.push_back(&this->dst_image->getBuffer(i));
        }

        this->image_copier->recordCopyImage(images,
                                            buffers);
    }


    const IFramebuffer& HostCopyingFramebufferChain::getFramebuffer(uint32_t index) const {
        return this->inner_framebuffer_chain.getFramebuffer(index);
    }

    const IFramebuffer& HostCopyingFramebufferChain::getCurrentFramebuffer() const {
        return this->inner_framebuffer_chain.getCurrentFramebuffer();
    }

    void HostCopyingFramebufferChain::swapToNextElement() {
        this->inner_framebuffer_chain.swapToNextElement();
        this->image_copier->runAndAwaitCopy();

        this->dst_image->setReadyForCopyFence(image_copier->getLastImageCopyCompleteFence());
    }

    uint32_t HostCopyingFramebufferChain::getElementChainLength() const {
        return this->inner_framebuffer_chain.getElementChainLength();
    }

    uint32_t HostCopyingFramebufferChain::getCurrentElementIndex() const {
        return this->inner_framebuffer_chain.getCurrentElementIndex();
    }

    void HostCopyingFramebufferChain::setPresentWaitSemaphores(WaitSemaphoreSet&& semaphores) {
        this->inner_framebuffer_chain.setPresentWaitSemaphores(std::move(semaphores));
    }

    SignalAndWaitSemaphores& HostCopyingFramebufferChain::getSemaphores() {
        throw std::runtime_error("[HostCopyingFramebufferChain] getSemaphores() not implemented");
    }
};
