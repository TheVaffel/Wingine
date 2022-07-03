#include "./BasicFramebufferSetup.hpp"

namespace wg::internal {

    BasicFramebufferSetup& BasicFramebufferSetup::setDepthOnly(bool enable) {
        this->depth_only = enable;
        return *this;
    }

    BasicFramebufferSetup& BasicFramebufferSetup::setSamplable(bool enable) {
        this->samplable = enable;
        return *this;
    }

    bool BasicFramebufferSetup::getDepthOnly() const {
        return this->depth_only;
    }

    bool BasicFramebufferSetup::getSamplable() const {
        return this->samplable;
    }
};
