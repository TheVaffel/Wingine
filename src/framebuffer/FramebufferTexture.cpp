#include "./FramebufferTexture.hpp"

#include "./BasicFramebuffer.hpp"
#include "../image/samplerUtil.hpp"

namespace wg::internal {
    FramebufferTexture::FramebufferTexture(const vk::Extent2D& dimensions,
                                           bool depth_only,
                                           std::shared_ptr<const DeviceManager> device_manager,
                                           CompatibleRenderPassRegistry& render_pass_registry)
        : device_manager(device_manager) {
        BasicFramebufferSetup setup;
        setup.setDepthOnly(depth_only);
        this->framebuffer = std::make_shared<BasicFramebuffer>(dimensions,
                                                               setup,
                                                               device_manager);
        this->sampler = samplerUtil::createBasicSampler({}, device_manager->getDevice());
    }


    vk::Extent2D FramebufferTexture::getDimensions() const {
        return this->framebuffer->getDimensions();
    }

    bool FramebufferTexture::hasColorImage() const {
        return this->framebuffer->hasColorImage();
    }

    const IImage& FramebufferTexture::getColorImage() const {
        return this->framebuffer->getColorImage();
    }

    bool FramebufferTexture::hasDepthImage() const {
        return this->framebuffer->hasDepthImage();
    }

    const IImage& FramebufferTexture::getDepthImage() const {
        return this->framebuffer->getDepthImage();
    }

    const vk::Framebuffer& FramebufferTexture::getFramebuffer() const {
        return this->framebuffer->getFramebuffer();
    }
};
