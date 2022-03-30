#pragma once

#include "./BasicFramebufferChain.hpp"

#include "./BasicFramebuffer.hpp"
#include "../DeviceManager.hpp"

namespace wg::internal {

    /*
     * Implementation
     */

    template<CFramebuffer T>
    template<typename ...Args>
    BasicFramebufferChain<T>::BasicFramebufferChain(uint32_t count, Args&&... arguments) {

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
        this->current_framebuffer = (this->current_framebuffer + 1) % this->framebuffers.size();
    }

};
