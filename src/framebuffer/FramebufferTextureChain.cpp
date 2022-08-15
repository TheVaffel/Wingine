#include "./FramebufferTextureChain.hpp"

#include "../sync/semaphoreUtil.hpp"

#include <iostream>

namespace wg::internal {
    FramebufferTextureChain::FramebufferTextureChain(uint32_t count,
                                                     const vk::Extent2D& dimensions,
                                                     bool depth_only,
                                                     std::shared_ptr<const DeviceManager> device_manager,
                                                     std::shared_ptr<const QueueManager> queue_manager,
                                                     CompatibleRenderPassRegistry& render_pass_registry)
    : ElementChainBase(count), semaphores(count, device_manager), queue_manager(queue_manager) {

        for (uint32_t i = 0; i < count; i++) {
            this->framebuffer_textures.push_back(std::make_shared<FramebufferTexture>(dimensions,
                                                                                      depth_only,
                                                                                      device_manager,
                                                                                      render_pass_registry));
        }
    }

    ITexture& FramebufferTextureChain::getTextureAt(uint32_t index) {
        return *this->framebuffer_textures[index];
    }

    const IFramebuffer& FramebufferTextureChain::getFramebuffer(uint32_t index) const {
        return *this->framebuffer_textures[index];
    }

    const IFramebuffer& FramebufferTextureChain::getCurrentFramebuffer() const {
        return *this->framebuffer_textures[this->getCurrentElementIndex()];
    }

    void FramebufferTextureChain::swapToNextElement() {
        this->ElementChainBase::swapToNextElement();
        semaphoreUtil::signalManySemaphoresFromManySemaphores(this->semaphores.getWaitSemaphores().getCurrentRawSemaphores(),
                                                              this->semaphores.getSignalSemaphores().getCurrentRawSemaphores(),
                                                              this->queue_manager->getGraphicsQueue());

        this->semaphores.getWaitSemaphores().swapSemaphores();
        this->semaphores.getSignalSemaphores().swapSemaphores();
    }

    IResource& FramebufferTextureChain::getResourceAt(uint32_t index) {
        return *this->framebuffer_textures[index];
    }

    SignalAndWaitSemaphores& FramebufferTextureChain::getSemaphores() {
        return this->semaphores;
    }

    void FramebufferTextureChain::setPresentWaitSemaphores(WaitSemaphoreSet&& semaphores) {
        this->semaphores.getWaitSemaphores().adoptFrom(std::move(semaphores));
    }

    SemaphoreChainPtr FramebufferTextureChain::addSignalImageAcquiredSemaphore() {
        return this->semaphores.getSignalSemaphores().addSignalledSemaphoreChain(this->queue_manager->getGraphicsQueue());
    }

    void FramebufferTextureChain::setSignalImageAcquiredSemaphores(SignalSemaphoreSet&& semaphores) {
        this->semaphores.getSignalSemaphores().adoptFrom(std::move(semaphores));
    }
};
