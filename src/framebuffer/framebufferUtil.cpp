#include "./framebufferUtil.hpp"

#include <flawed_assert.hpp>

#include <array>

namespace wg::internal::framebufferUtil {

    vk::Framebuffer createBasicFramebuffer(const IImage& colorImage,
                                           const IImage& depthImage,
                                           CompatibleRenderPassRegistry& render_pass_registry,
                                           const vk::Device& device) {
        fl_assert_eq(colorImage.getDimensions().width, depthImage.getDimensions().width);
        fl_assert_eq(colorImage.getDimensions().height, depthImage.getDimensions().height);

        vk::RenderPass render_pass =
            render_pass_registry.ensureAndGetRenderPass(renderPassUtil::RenderPassType::colorDepth);

        std::array<vk::ImageView, 2> attachments = { colorImage.getView(), depthImage.getView() };

        vk::FramebufferCreateInfo fci;
        fci.setAttachments(attachments)
            .setWidth(colorImage.getDimensions().width)
            .setHeight(colorImage.getDimensions().height)
            .setRenderPass(render_pass)
            .setAttachments(attachments)
            .setLayers(1);

        return device.createFramebuffer(fci);
    }
};
