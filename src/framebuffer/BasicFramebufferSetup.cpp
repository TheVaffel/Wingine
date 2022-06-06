#include "./BasicFramebufferSetup.hpp"

namespace wg::internal {

    BasicFramebufferSetup& BasicFramebufferSetup::setDepthOnly(bool enable) {
        this->depth_only = enable;
        return *this;
    }

    bool BasicFramebufferSetup::getDepthOnly() const {
        return this->depth_only;
    }
};
