#pragma once


#include "./BasicFramebufferSetup.hpp"
#include "./IFramebuffer.hpp"

#include "../image/BasicImage.hpp"
#include "../SwapchainManager.hpp"

#include "../render_pass/CompatibleRenderPassRegistry.hpp"

namespace wg::internal {
    class BasicFramebuffer : public IFramebuffer {

        std::optional<BasicImage> color_image;
        BasicImage depth_image;

        vk::Framebuffer framebuffer;

        std::shared_ptr<const DeviceManager> device_manager;

        void initializeColorImage(const vk::Extent2D& dimensions,
                                  const BasicFramebufferSetup& setup);
        void initializeDepthImage(const vk::Extent2D& dimensions,
                                  const BasicFramebufferSetup& setup);

    public:
        BasicFramebuffer(const vk::Extent2D& dimensions,
                         const BasicFramebufferSetup& setup,
                         std::shared_ptr<const DeviceManager> device_manager,
                         CompatibleRenderPassRegistry& render_pass_registry);


        virtual vk::Extent2D getDimensions() const final;

        virtual const vk::Framebuffer& getFramebuffer() const final;

        virtual bool hasColorImage() const final;
        virtual const IImage& getColorImage() const final;

        virtual bool hasDepthImage() const final;
        virtual const IImage& getDepthImage() const final;

        ~BasicFramebuffer();
    };
};
