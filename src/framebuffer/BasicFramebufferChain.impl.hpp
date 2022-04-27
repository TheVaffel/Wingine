#pragma once

#include "./BasicFramebufferChain.hpp"

#include "./BasicFramebuffer.hpp"
#include "../DeviceManager.hpp"
#include "../sync/semaphoreUtil.hpp"

#include <iostream>

namespace wg::internal {

    /*
     * Implementation
     */

    template<CFramebuffer T>
    template<typename ...Args>
    BasicFramebufferChain<T>::BasicFramebufferChain(uint32_t count,
                                                    std::shared_ptr<const DeviceManager> device_manager,
                                                    std::shared_ptr<const QueueManager> queue_manager,
                                                    Args&&... arguments)
        : device_manager(device_manager),
          queue_manager(queue_manager),
          wait_semaphore_set(count, device_manager),
          signal_semaphore_set(count, device_manager) {
        this->framebuffers.reserve(count);
        this->current_framebuffer = 0;

        for (uint32_t i = 0; i < count; i++) {
            this->framebuffers.push_back(std::make_unique<T>(arguments...));
        }
    }

    template<CFramebuffer T>
    uint32_t BasicFramebufferChain<T>::getNumFramebuffers() const {
        return this->framebuffers.size();
    }

    template<CFramebuffer T>
    const IFramebuffer& BasicFramebufferChain<T>::getFramebuffer(uint32_t index) const {
        return *this->framebuffers[index];
    }

    template<CFramebuffer T>
    IFramebuffer& BasicFramebufferChain<T>::getFramebuffer(uint32_t index) {
        return *this->framebuffers[index];
    }

    template<CFramebuffer T>
    const IFramebuffer& BasicFramebufferChain<T>::getCurrentFramebuffer() const {
        return *this->framebuffers[this->current_framebuffer];
    }

    template<CFramebuffer T>
    IFramebuffer& BasicFramebufferChain<T>::getCurrentFramebuffer() {
        return *this->framebuffers[this->current_framebuffer];
    }

    template<CFramebuffer T>
    void BasicFramebufferChain<T>::swapFramebuffer() {
        semaphoreUtil::signalManySemaphoresFromManySemaphores(this->wait_semaphore_set.getCurrentRawSemaphores(),
                                                              this->signal_semaphore_set.getCurrentRawSemaphores(),
                                                              this->queue_manager->getGraphicsQueue());

        this->wait_semaphore_set.swapSemaphores();
        this->signal_semaphore_set.swapSemaphores();

        this->current_framebuffer = (this->current_framebuffer + 1) % this->framebuffers.size();
    }

    template<CFramebuffer T>
    void BasicFramebufferChain<T>::setPresentWaitSemaphores(const WaitSemaphoreSet& semaphores) {
        this->wait_semaphore_set = semaphores;
    }

    template<CFramebuffer T>
    std::shared_ptr<ManagedSemaphoreChain> BasicFramebufferChain<T>::addSignalImageAcquiredSemaphore() {
        return this->signal_semaphore_set.addSignalledSemaphoreChain(this->queue_manager->getGraphicsQueue());
    }

    template<CFramebuffer T>
    void BasicFramebufferChain<T>::setSignalImageAcquiredSemaphores(const SignalSemaphoreSet& semaphores) {
        this->signal_semaphore_set = semaphores;
    }
};
