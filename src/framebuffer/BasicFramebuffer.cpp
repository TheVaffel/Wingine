#include "./BasicFramebuffer.hpp"

#include "../image/BasicImage.hpp"
#include "./framebufferUtil.hpp"

namespace wg::internal {

    BasicFramebuffer::BasicFramebuffer(const vk::Extent2D& dimensions,
                                       const BasicFramebufferSetup& setup,
                                       std::shared_ptr<const DeviceManager> device_manager,
                                       CompatibleRenderPassRegistry& render_pass_registry)
        : device_manager(device_manager) {

        if (!setup.getDepthOnly()) {
            this->color_image = BasicImage::createFramebufferColorImage(dimensions,
                                                                        device_manager);
        }

        this->depth_image = BasicImage::createFramebufferDepthImage(dimensions,
                                                                    device_manager);

        this->framebuffer = framebufferUtil::createBasicFramebuffer(*this->color_image,
                                                                    *this->depth_image,
                                                                    render_pass_registry,
                                                                    device_manager->getDevice());
    }

    vk::Extent2D BasicFramebuffer::getDimensions() const {
        return this->color_image->getDimensions();
    }

    bool BasicFramebuffer::hasColorImage() const {
        return !!this->color_image;
    }

    const IImage& BasicFramebuffer::getColorImage() const {
        if (!this->color_image) {
            throw std::runtime_error("[BasicFramebuffer] Not created with color image");
        }
        return *this->color_image;
    }

    bool BasicFramebuffer::hasDepthImage() const {
        return true;
    }

    const IImage& BasicFramebuffer::getDepthImage() const {
        return *this->depth_image;
    }

    const vk::Framebuffer& BasicFramebuffer::getFramebuffer() const {
        return this->framebuffer;
    }

    BasicFramebuffer::~BasicFramebuffer() {
        this->device_manager->getDevice().destroy(this->framebuffer);
    }
};
