#include "./BasicImageSettings.hpp"

#include "./imageUtil.hpp"

namespace wg::internal {
    BasicImageSettings::BasicImageSettings(vk::ImageAspectFlagBits aspect,
                                           vk::ImageLayout intended_layout,
                                           vk::ImageUsageFlags usage)
        : aspect(aspect), intended_layout(intended_layout), usage(usage) {
        this->format = this->aspect == vk::ImageAspectFlagBits::eColor ?
            imageUtil::DEFAULT_FRAMEBUFFER_COLOR_IMAGE_FORMAT :
            imageUtil::DEFAULT_FRAMEBUFFER_DEPTH_IMAGE_FORMAT;
    }

    BasicImageSettings BasicImageSettings::createFramebufferColorImageSettings() {
        return BasicImageSettings(vk::ImageAspectFlagBits::eColor,
                                  vk::ImageLayout::ePresentSrcKHR,
                                  vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc);
    }

    BasicImageSettings BasicImageSettings::createFramebufferDepthImageSettings() {
        return BasicImageSettings(vk::ImageAspectFlagBits::eDepth,
                                  vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                  vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc);

    }

    BasicImageSettings BasicImageSettings::createFramebufferTextureColorImageSettings() {
        return BasicImageSettings(vk::ImageAspectFlagBits::eColor,
                                  vk::ImageLayout::eColorAttachmentOptimal,
                                  vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);
    }

    BasicImageSettings BasicImageSettings::createFramebufferTextureDepthImageSettings() {
        return BasicImageSettings(vk::ImageAspectFlagBits::eDepth,
                                  vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                  vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled);
    }

    BasicImageSettings BasicImageSettings::createHostAccessibleColorImageSettings() {
        BasicImageSettings settings(vk::ImageAspectFlagBits::eColor,
                                    vk::ImageLayout::eGeneral,
                                    vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst);
        settings.format = vk::Format::eB8G8R8A8Unorm;
        settings.tiling = vk::ImageTiling::eLinear;
        settings.with_view = false;
        settings.memory_properties = vk::MemoryPropertyFlagBits::eHostVisible;
        return settings;
    }

    BasicImageSettings BasicImageSettings::createColorTextureImageSettings() {
        return BasicImageSettings(vk::ImageAspectFlagBits::eColor,
                                  vk::ImageLayout::eShaderReadOnlyOptimal,
                                  vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst);
    }
};
