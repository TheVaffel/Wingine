#pragma once

#include "./IFramebuffer.hpp"
#include "../image/BasicImage.hpp"

#include "../render_pass/CompatibleRenderPassRegistry.hpp"

namespace wg::internal {

    class DepthOnlyFramebuffer : public IFramebuffer {

        std::shared_ptr<const DeviceManager> device_manager;

        std::unique_ptr<IImage> depth_image;
        vk::Framebuffer framebuffer;

    public:

        DepthOnlyFramebuffer(const vk::Extent2D& dimensions,
                             std::shared_ptr<const DeviceManager> device_manager,
                             CompatibleRenderPassRegistry& render_pass_registry);

        virtual vk::Extent2D getDimensions() const;

        virtual bool hasColorImage() const;
        virtual const IImage& getColorImage() const;
        virtual IImage& getColorImage();

        virtual bool hasDepthImage() const;
        virtual const IImage& getDepthImage() const;
        virtual IImage& getDepthImage();

        virtual const vk::Framebuffer& getFramebuffer() const;

        ~DepthOnlyFramebuffer();
    };
};
