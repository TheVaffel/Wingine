#include "./BasicImageSetup.hpp"

#include "./imageUtil.hpp"

namespace wg::internal {
    BasicImageSetup::BasicImageSetup(vk::ImageAspectFlagBits aspect,
                                           vk::ImageLayout intended_layout,
                                           vk::ImageUsageFlags usage)
        : aspect(aspect), intended_layout(intended_layout), usage(usage) {
        this->format = this->aspect == vk::ImageAspectFlagBits::eColor ?
            imageUtil::DEFAULT_FRAMEBUFFER_COLOR_IMAGE_FORMAT :
            imageUtil::DEFAULT_FRAMEBUFFER_DEPTH_IMAGE_FORMAT;
    }

    BasicImageSetup BasicImageSetup::createFramebufferColorImageSetup() {
        return BasicImageSetup(vk::ImageAspectFlagBits::eColor,
                                  vk::ImageLayout::ePresentSrcKHR,
                                  vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc);
    }

    BasicImageSetup BasicImageSetup::createFramebufferDepthImageSetup() {
        return BasicImageSetup(vk::ImageAspectFlagBits::eDepth,
                                  vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                  vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc);

    }

    BasicImageSetup BasicImageSetup::createFramebufferTextureColorImageSetup() {
        return BasicImageSetup(vk::ImageAspectFlagBits::eColor,
                                  vk::ImageLayout::eColorAttachmentOptimal,
                                  vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);
    }

    BasicImageSetup BasicImageSetup::createFramebufferTextureDepthImageSetup() {
        return BasicImageSetup(vk::ImageAspectFlagBits::eDepth,
                                  vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                  vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled);
    }

    BasicImageSetup BasicImageSetup::createHostAccessibleColorImageSetup() {
        BasicImageSetup settings(vk::ImageAspectFlagBits::eColor,
                                    vk::ImageLayout::eGeneral,
                                    vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst);
        settings.format = vk::Format::eB8G8R8A8Unorm;
        settings.tiling = vk::ImageTiling::eLinear;
        settings.with_view = false;
        settings.memory_properties = vk::MemoryPropertyFlagBits::eHostVisible;
        return settings;
    }

    BasicImageSetup BasicImageSetup::createColorTextureImageSetup() {
        return BasicImageSetup(vk::ImageAspectFlagBits::eColor,
                                  vk::ImageLayout::eShaderReadOnlyOptimal,
                                  vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst);
    }


    BasicImageSetup BasicImageSetup::createStorageTextureSetup() {
        return BasicImageSetup(vk::ImageAspectFlagBits::eColor,
                                  vk::ImageLayout::eGeneral,
                                  vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage);

    }
};
