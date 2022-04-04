#include "./renderPassUtil.hpp"

#include "../image/imageUtil.hpp"

#include <tuple>
#include <optional>

namespace wg::internal::renderPassUtil {

    namespace {
        vk::AttachmentDescription getAttachmentDescription(bool clear_on_load,
                                                           vk::ImageLayout layout,
                                                           vk::Format format) {
            vk::AttachmentDescription desc;
            desc.setLoadOp(clear_on_load ?
                           vk::AttachmentLoadOp::eClear :
                           vk::AttachmentLoadOp::eLoad)
                .setStoreOp(vk::AttachmentStoreOp::eStore)
                .setStencilLoadOp(vk::AttachmentLoadOp::eLoad)
                .setStencilStoreOp(vk::AttachmentStoreOp::eStore)
                .setInitialLayout(clear_on_load ?
                                  vk::ImageLayout::eUndefined :
                                  layout)
                .setFinalLayout(layout)
                .setFormat(format);

            return desc;
        }

        vk::AttachmentDescription getColorAttachmentDescription(bool clear_on_load) {
            return getAttachmentDescription(clear_on_load,
                                            vk::ImageLayout::ePresentSrcKHR,
                                            imageUtil::DEFAULT_FRAMEBUFFER_COLOR_IMAGE_FORMAT);
        }

        vk::AttachmentDescription getDepthAttachmentDescription(bool clear_on_load) {
            return getAttachmentDescription(clear_on_load,
                                            vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                            imageUtil::DEFAULT_FRAMEBUFFER_DEPTH_IMAGE_FORMAT);
        }

        std::vector<vk::AttachmentDescription> getColorAttachmentDescriptions(const std::vector<bool>& clears) {
            std::vector<vk::AttachmentDescription> descriptions(clears.size());

            for (uint32_t i = 0; i < descriptions.size(); i++) {
                descriptions[i] = getColorAttachmentDescription(clears[i]);
            }

            return descriptions;
        }

        std::vector<vk::AttachmentReference> getColorAttachmentReferences(uint32_t num_references,
                                                                          uint32_t binding_offset = 0) {
            std::vector<vk::AttachmentReference> references(num_references);

            for (uint32_t i = 0; i < num_references; i++) {
                references[i] = vk::AttachmentReference(binding_offset + i,
                                                        vk::ImageLayout::eColorAttachmentOptimal);
            }

            return references;
        }

        vk::AttachmentReference getDepthAttachmentReference(uint32_t binding_offset) {
            return vk::AttachmentReference(binding_offset, vk::ImageLayout::eDepthStencilAttachmentOptimal);
        }

        vk::SubpassDescription createSubpassDescription(const std::vector<vk::AttachmentReference>& color_references,
                                                        const std::optional<vk::AttachmentReference>& depth_stencil_reference) {
            vk::SubpassDescription spd;
            spd.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                .setColorAttachments(color_references)
                .setPDepthStencilAttachment(depth_stencil_reference.has_value() ?
                                            &depth_stencil_reference.value() :
                                            nullptr);
            return spd;
        }

        vk::RenderPass createRenderPass(const std::vector<vk::AttachmentDescription>& descriptions,
                                        const vk::SubpassDescription& subpass_description,
                                        const vk::Device& device) {

            vk::RenderPassCreateInfo rpci;
            rpci.setAttachmentCount(descriptions.size())
                .setPAttachments(descriptions.data())
                .setSubpassCount(1)
                .setPSubpasses(&subpass_description);

            return device.createRenderPass(rpci);
        }
    };

    vk::RenderPass createDefaultRenderPass(RenderPassType type,
                                           const vk::Device& device) {
        RenderPassSetup setup;

        switch (type) {
        case RenderPassType::colorDepth:
            setup.setColorClears({true});
            setup.setDepthClear(true);
            break;
        case RenderPassType::depthOnly:
            setup.setColorClears({});
            setup.setDepthClear(true);
            break;
        };

        return createRenderPass(setup, device);
    }


    /*
     * RenderPassSetup
     */

    uint32_t RenderPassSetup::getNumColorAttachments() const {
        return this->color_clears.size();
    }

    const std::vector<bool>& RenderPassSetup::getColorClears() const {
        return this->color_clears;
    }

    bool RenderPassSetup::hasDepthAttachment() const {
        return this->depth_clear.has_value();
    }

    bool RenderPassSetup::getDepthClear() const {
        if (!this->depth_clear.has_value()) {
            throw std::runtime_error("[RenderPassSetup] Tried to get depth clear on render pass setup without depth");
        }
        return *this->depth_clear;
    }

    RenderPassSetup& RenderPassSetup::setNumColorAttachments(uint32_t num_color_attachments) {
        this->color_clears = std::vector<bool>(num_color_attachments, true);
        return *this;
    }

    RenderPassSetup& RenderPassSetup::setColorClears(const std::vector<bool>& color_clears) {
        this->color_clears = color_clears;
        return *this;
    }

    RenderPassSetup& RenderPassSetup::setEnableDepthAttachment(bool enable) {
        if (enable) {
            this->depth_clear = true;
        } else {
            this->depth_clear.reset();
        }

        return *this;
    }

    RenderPassSetup& RenderPassSetup::setDepthClear(bool enable) {
        this->depth_clear = enable;
        return *this;
    }

    /*
     * Generic create function
     */

    vk::RenderPass createRenderPass(const RenderPassSetup& setup, const vk::Device& device) {
        std::vector<vk::AttachmentDescription> attachment_descriptions =
            getColorAttachmentDescriptions(setup.getColorClears());

        std::vector<vk::AttachmentReference> color_references =
            getColorAttachmentReferences(setup.getNumColorAttachments());

        std::optional<vk::AttachmentReference> depth_reference;

        if (setup.hasDepthAttachment()) {
            attachment_descriptions.push_back(getDepthAttachmentDescription(setup.getDepthClear()));
            depth_reference = getDepthAttachmentReference(color_references.size());
        }

        vk::SubpassDescription subpass_description = createSubpassDescription(color_references, depth_reference);

        return createRenderPass(attachment_descriptions, subpass_description, device);
    }
};
