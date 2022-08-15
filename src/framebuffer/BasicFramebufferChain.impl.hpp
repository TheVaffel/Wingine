#pragma once

#include "./BasicFramebufferChain.hpp"

#include "./BasicFramebuffer.hpp"
#include "../core/DeviceManager.hpp"
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
        : FramebufferChainBase(count, queue_manager, device_manager),
          framebuffer_index(count) {

        this->framebuffers.reserve(count);

        for (uint32_t i = 0; i < count; i++) {
            this->framebuffers.push_back(std::make_unique<T>(arguments...));
        }
    }

    template<CFramebuffer T>
    const IFramebuffer& BasicFramebufferChain<T>::getFramebuffer(uint32_t index) const {
        return *this->framebuffers[index];
    }

    template<CFramebuffer T>
    const IFramebuffer& BasicFramebufferChain<T>::getCurrentFramebuffer() const {
        return *this->framebuffers[this->framebuffer_index.getCurrentIndex()];
    }

    template<CFramebuffer T>
    uint32_t BasicFramebufferChain<T>::getElementChainLength() const {
        return this->framebuffers.size();
    }

    template<CFramebuffer T>
    uint32_t BasicFramebufferChain<T>::getCurrentElementIndex() const {
        return this->framebuffer_index.getCurrentIndex();
    }

    template<CFramebuffer T>
    void BasicFramebufferChain<T>::swapToNextElement() {
        semaphoreUtil::signalManySemaphoresFromManySemaphores(this->getWaitSemaphores().getCurrentRawSemaphores(),
                                                              this->getSignalSemaphores().getCurrentRawSemaphores(),
                                                              this->queue_manager->getGraphicsQueue());

        this->getWaitSemaphores().swapSemaphores();
        this->getSignalSemaphores().swapSemaphores();

        this->framebuffer_index.incrementIndex();
    }
};
