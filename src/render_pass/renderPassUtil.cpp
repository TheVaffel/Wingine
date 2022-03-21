#include "./renderPassUtil.hpp"

#include "../image/imageUtil.hpp"

#include <tuple>
#include <optional>

namespace wg::internal::renderPassUtil {

    namespace {
        struct RenderPassSetup {
            RenderPassType type;
            bool clear_color_on_load = false;
            bool clear_depth_on_load = false;

            RenderPassSetup& setClear(bool enable_clear) {
                this->clear_color_on_load = enable_clear;
                this->clear_depth_on_load = enable_clear;
                return *this;
            }

            RenderPassSetup(RenderPassType type) : type(type) { }
        };

        vk::AttachmentDescription getColorAttachmentDescription(const RenderPassSetup& renderPassSetup) {
            vk::AttachmentDescription desc;
            desc.setLoadOp(renderPassSetup.clear_color_on_load ?
                           vk::AttachmentLoadOp::eClear :
                           vk::AttachmentLoadOp::eLoad)
                .setStoreOp(vk::AttachmentStoreOp::eStore)
                .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                .setInitialLayout(renderPassSetup.clear_color_on_load ?
                                  vk::ImageLayout::eUndefined :
                                  vk::ImageLayout::ePresentSrcKHR)
                .setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
                .setFormat(imageUtil::DEFAULT_FRAMEBUFFER_COLOR_IMAGE_FORMAT);

            return desc;
        }

        vk::AttachmentDescription getDepthAttachmentDescription(const RenderPassSetup& render_pass_setup) {
            vk::AttachmentDescription desc;
            desc.setLoadOp(render_pass_setup.clear_depth_on_load ?
                           vk::AttachmentLoadOp::eClear :
                           vk::AttachmentLoadOp::eLoad)
                .setStoreOp(vk::AttachmentStoreOp::eStore)
                .setStencilLoadOp(vk::AttachmentLoadOp::eLoad)
                .setStencilStoreOp(vk::AttachmentStoreOp::eStore)
                .setInitialLayout(render_pass_setup.clear_depth_on_load ?
                                  vk::ImageLayout::eUndefined :
                                  vk::ImageLayout::eDepthStencilAttachmentOptimal)
                .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
                .setFormat(imageUtil::DEFAULT_FRAMEBUFFER_DEPTH_IMAGE_FORMAT);

            return desc;
        }

        std::tuple<std::vector<vk::AttachmentDescription>,
                   std::vector<vk::AttachmentReference>,
                   std::optional<vk::AttachmentReference>>
        getColorAndDepthAttachmentDescriptionsAndReferences(const RenderPassSetup& render_pass_setup) {
            return std::make_tuple<std::vector<vk::AttachmentDescription>,
                                   std::vector<vk::AttachmentReference>,
                                   std::optional<vk::AttachmentReference>>
                ({ getColorAttachmentDescription(render_pass_setup),
                     getDepthAttachmentDescription(render_pass_setup) },
                    { vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal) },
                    { vk::AttachmentReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal) });
        }

        vk::SubpassDescription createSimpleSubpassDescription(const std::vector<vk::AttachmentDescription>& descriptions,
                                                              const std::vector<vk::AttachmentReference>& color_references,
                                                              const std::optional<vk::AttachmentReference>& depth_stencil_reference) {
            vk::SubpassDescription spd;
            spd.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                .setColorAttachments(color_references)
                .setPDepthStencilAttachment(depth_stencil_reference ?
                                            &depth_stencil_reference.value() :
                                            nullptr);
            return spd;
        }


        vk::RenderPass createRenderPassRaw(const RenderPassSetup& renderPassSetup,
                                           const vk::Device& device) {

            std::vector<vk::AttachmentDescription> descriptions;
            std::vector<vk::AttachmentReference> color_references;
            std::optional<vk::AttachmentReference> depth_stencil_reference;

            switch (renderPassSetup.type) {

            case RenderPassType::colorDepth:

                std::tie(descriptions, color_references, depth_stencil_reference) =
                    getColorAndDepthAttachmentDescriptionsAndReferences(renderPassSetup);
                break;

            case RenderPassType::depthOnly:

                descriptions = { getDepthAttachmentDescription(renderPassSetup) };
                depth_stencil_reference = { vk::AttachmentReference(0, vk::ImageLayout::eDepthStencilAttachmentOptimal) };
                break;
            }

            vk::SubpassDescription spd = createSimpleSubpassDescription(descriptions,
                                                                        color_references,
                                                                        depth_stencil_reference);

            vk::RenderPassCreateInfo rpci;
            rpci.setAttachmentCount(descriptions.size())
                .setPAttachments(descriptions.data())
                .setSubpassCount(1)
                .setPSubpasses(&spd);

            return device.createRenderPass(rpci);
        }
    };

    vk::RenderPass createDefaultRenderPass(RenderPassType type,
                                           const vk::Device& device) {

        RenderPassSetup setup(type);
        return createRenderPassRaw(setup, device);
    }
};
