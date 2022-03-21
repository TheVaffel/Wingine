#pragma once

#include "../image/IImage.hpp"

namespace wg::internal {
    class IFramebuffer {
    public:

        virtual bool hasColorImage() const = 0;
        virtual const IImage& getColorImage() const = 0;
        virtual IImage& getColorImage() = 0;

        virtual bool hasDepthImage() const = 0;
        virtual const IImage& getDepthImage() const = 0;
        virtual IImage& getDepthImage() = 0;

        virtual const vk::Framebuffer& getFramebuffer() const = 0;

        virtual ~IFramebuffer();
    };
};
