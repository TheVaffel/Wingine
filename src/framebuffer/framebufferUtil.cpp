#include "./framebufferUtil.hpp"

#include <flawed_assert.hpp>

#include <array>

namespace wg::internal::framebufferUtil {

    vk::Framebuffer createBasicFramebuffer(const IImage& color_image,
                                           const IImage& depth_image,
                                           CompatibleRenderPassRegistry& render_pass_registry,
                                           const vk::Device& device) {
        fl_assert_eq(color_image.getDimensions().width, depth_image.getDimensions().width);
        fl_assert_eq(color_image.getDimensions().height, depth_image.getDimensions().height);

        vk::RenderPass render_pass =
            render_pass_registry.ensureAndGetRenderPass(renderPassUtil::RenderPassType::colorDepth);

        std::array<vk::ImageView, 2> attachments = { color_image.getView(), depth_image.getView() };

        vk::FramebufferCreateInfo fci;
        fci.setAttachments(attachments)
            .setWidth(color_image.getDimensions().width)
            .setHeight(color_image.getDimensions().height)
            .setRenderPass(render_pass)
            .setLayers(1);

        return device.createFramebuffer(fci);
    }

    vk::Framebuffer createDepthOnlyFramebuffer(const IImage& depth_image,
                                               CompatibleRenderPassRegistry& render_pass_registry,
                                               const vk::Device& device) {
        vk::RenderPass render_pass =
            render_pass_registry.ensureAndGetRenderPass(renderPassUtil::RenderPassType::depthOnly);

        std::array<vk::ImageView, 1> attachments = { depth_image.getView() };

        vk::FramebufferCreateInfo fci;
        fci.setAttachments(attachments)
            .setWidth(depth_image.getDimensions().width)
            .setHeight(depth_image.getDimensions().height)
            .setRenderPass(render_pass)
            .setLayers(1);
        return device.createFramebuffer(fci);
    }
};
