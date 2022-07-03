#pragma once

#include "./IFramebuffer.hpp"
#include "../image/ITexture.hpp"
#include "../render_pass/CompatibleRenderPassRegistry.hpp"

#include "../DeviceManager.hpp"

namespace wg::internal {
    class FramebufferTexture : public IFramebuffer, public ITexture {
        std::shared_ptr<const DeviceManager> device_manager;

        vk::Sampler sampler;
        std::shared_ptr<IFramebuffer> framebuffer;

        const IImage& getRelevantFramebufferImage() const;
        IImage& getRelevantFramebufferImage();

    public:

        FramebufferTexture(const vk::Extent2D& dimensions,
                           bool depth_only,
                           std::shared_ptr<const DeviceManager> device_manager,
                           CompatibleRenderPassRegistry& render_pass_registry);

        virtual vk::Extent2D getDimensions() const override;

        virtual bool hasColorImage() const override;
        virtual const IImage& getColorImage() const override;

        virtual bool hasDepthImage() const override;
        virtual const IImage& getDepthImage() const override;

        virtual vk::Sampler getSampler() const override;

        virtual const vk::Framebuffer& getFramebuffer() const override;

        virtual const vk::Image getImage() const override;
        virtual const vk::DeviceMemory getMemory() const override;
        virtual const vk::ImageView getView() const override;

        virtual const vk::ImageAspectFlagBits getDefaultAspect() const override;

        virtual const vk::ImageLayout getIntendedLayout() const override;
        virtual void setCurrentLayout(const vk::ImageLayout& layout) override;

        [[nodiscard]]
        virtual std::unique_ptr<IResourceWriteAuxillaryData>
        writeDescriptorUpdate(vk::WriteDescriptorSet& write_info) const override;

        ~FramebufferTexture();
    };
};
