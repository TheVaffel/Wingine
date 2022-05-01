#include "./IFramebufferChain.hpp"

namespace wg::internal {
    IFramebufferChain::~IFramebufferChain() { }

    IFramebuffer& IFramebufferChain::getCurrentFramebuffer() {
        return const_cast<IFramebuffer&>(static_cast<IFramebufferChain const &>(*this).getCurrentFramebuffer());
    }

    IFramebuffer& IFramebufferChain::getFramebuffer(uint32_t index) {
        return const_cast<IFramebuffer&>(static_cast<IFramebufferChain const &>(*this).getFramebuffer(index));
    }
};
