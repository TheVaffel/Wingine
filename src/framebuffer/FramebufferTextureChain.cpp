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
    : framebuffer_index(count), semaphores(count, device_manager), queue_manager(queue_manager) {

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

    uint32_t FramebufferTextureChain::getNumFramebuffers() const {
        return this->framebuffer_textures.size();
    }

    const IFramebuffer& FramebufferTextureChain::getFramebuffer(uint32_t index) const {
        return *this->framebuffer_textures[index];
    }

    const IFramebuffer& FramebufferTextureChain::getCurrentFramebuffer() const {
        return *this->framebuffer_textures[this->framebuffer_index.getCurrentIndex()];
    }

    void FramebufferTextureChain::swapFramebuffer() {
        semaphoreUtil::signalManySemaphoresFromManySemaphores(this->semaphores.getWaitSemaphores().getCurrentRawSemaphores(),
                                                              this->semaphores.getSignalSemaphores().getCurrentRawSemaphores(),
                                                              this->queue_manager->getGraphicsQueue());

        this->semaphores.getWaitSemaphores().swapSemaphores();
        this->semaphores.getSignalSemaphores().swapSemaphores();
        this->framebuffer_index.incrementIndex();
    }

    void FramebufferTextureChain::swap() {
        throw std::runtime_error("[FramebufferTextureChain] Should not use swap() on FramebufferTextureChain");
    }

    uint32_t FramebufferTextureChain::getCurrentIndex() const {
        return this->framebuffer_index.getCurrentIndex();
    }

    uint32_t FramebufferTextureChain::getNumResources() const {
        return this->framebuffer_textures.size();
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
