#include "./IFramebuffer.hpp"

namespace wg::internal {
    IFramebuffer::~IFramebuffer() { }

    IImage& IFramebuffer::getColorImage() {
        return const_cast<IImage&>(static_cast<IFramebuffer const &>(*this).getColorImage());
    }

    IImage& IFramebuffer::getDepthImage() {
        return const_cast<IImage&>(static_cast<IFramebuffer const &>(*this).getDepthImage());
    }
};
