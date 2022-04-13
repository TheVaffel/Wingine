#pragma once

#include "./IFramebuffer.hpp"

#include "../image/IImage.hpp"
#include "../SwapchainManager.hpp"

#include "../render_pass/CompatibleRenderPassRegistry.hpp"

namespace wg::internal {
    class BasicFramebuffer : public IFramebuffer {
        std::unique_ptr<IImage> color_image;
        std::unique_ptr<IImage> depth_image;
        vk::Framebuffer framebuffer;

        std::shared_ptr<const DeviceManager> device_manager;

    public:
        BasicFramebuffer(const vk::Extent2D& dimensions,
                         std::shared_ptr<const DeviceManager> device_manager,
                         CompatibleRenderPassRegistry& render_pass_registry);


        virtual vk::Extent2D getDimensions() const;

        virtual const vk::Framebuffer& getFramebuffer() const;

        virtual bool hasColorImage() const;
        virtual const IImage& getColorImage() const;
        virtual IImage& getColorImage();

        virtual bool hasDepthImage() const;
        virtual const IImage& getDepthImage() const;
        virtual IImage& getDepthImage();

        ~BasicFramebuffer();
    };
};
