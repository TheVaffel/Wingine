#pragma once

#include "../image/IImage.hpp"

namespace wg::internal {
    class IFramebuffer : public virtual IDimensionable {
    public:

        virtual bool hasColorImage() const = 0;
        virtual const IImage& getColorImage() const = 0;
        IImage& getColorImage();

        virtual bool hasDepthImage() const = 0;
        virtual const IImage& getDepthImage() const = 0;
        IImage& getDepthImage();

        virtual const vk::Framebuffer& getFramebuffer() const = 0;

        virtual ~IFramebuffer();
    };
};
