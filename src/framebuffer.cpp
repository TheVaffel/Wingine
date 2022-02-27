#include "./framebuffer.hpp"

#include "./Wingine.hpp"
#include "./CompatibleRenderPassRegistry.hpp"

namespace wg {
    Framebuffer::Framebuffer() { }

    Framebuffer::Framebuffer(Wingine& wing,
                             int width, int height,
                             bool depthOnly) {

        std::vector<vk::ImageView> attachments;

        if (!depthOnly) {
            wing.cons_image_image(this->colorImage,
                                  width, height,
                                  vk::Format::eB8G8R8A8Unorm,
                                  vk::ImageUsageFlagBits::eColorAttachment |
                                  vk::ImageUsageFlagBits::eTransferSrc,
                                  vk::ImageTiling::eOptimal);
            wing.cons_image_memory(this->colorImage,
                                   vk::MemoryPropertyFlagBits::eDeviceLocal);
            wing.cons_image_view(this->colorImage,
                                 wImageViewColor,
                                 vk::Format::eB8G8R8A8Unorm);

            attachments.push_back(this->colorImage.view);
        }


        wing.cons_image_image(this->depthImage,
                              width, height,
                              vk::Format::eD32Sfloat,
                              vk::ImageUsageFlagBits::eDepthStencilAttachment |
                              vk::ImageUsageFlagBits::eTransferSrc,
                              vk::ImageTiling::eOptimal);

        wing.cons_image_memory(this->depthImage,
                               vk::MemoryPropertyFlagBits::eDeviceLocal);
        wing.cons_image_view(this->depthImage,
                             wImageViewDepth,
                             vk::Format::eD32Sfloat);

        attachments.push_back(this->depthImage.view);

        internal::RenderPassType render_pass_type =
            depthOnly ?
            internal::RenderPassType::renDepth :
            internal::RenderPassType::renColorDepth;

        if (!wing.compatibleRenderPassRegistry->hasRenderPassType(render_pass_type)) {
            wing.register_compatible_render_pass(render_pass_type);
        }

        vk::FramebufferCreateInfo finf;
        finf.setRenderPass(wing.compatibleRenderPassRegistry->getRenderPass(render_pass_type))
            .setAttachmentCount(attachments.size())
            .setPAttachments(attachments.data())
            .setWidth(width)
            .setHeight(height)
            .setLayers(1);

        vk::Device device = wing.getDevice();

        this->framebuffer = device.createFramebuffer(finf);
    }


    const vk::Framebuffer& Framebuffer::getFramebuffer() const {
        return this->framebuffer;
    }
};
