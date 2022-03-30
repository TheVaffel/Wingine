#include "./BasicFramebuffer.hpp"

#include "../image/BasicImage.hpp"
#include "./framebufferUtil.hpp"

namespace wg::internal {

    BasicFramebuffer::BasicFramebuffer(const vk::Extent2D& dimensions,
                                       std::shared_ptr<const DeviceManager> device_manager,
                                       CompatibleRenderPassRegistry& render_pass_registry)
        : device_manager(device_manager) {

        this->color_image = BasicImage::createFramebufferColorImage(dimensions,
                                                                    device_manager);

        this->depth_image = BasicImage::createFramebufferDepthImage(dimensions,
                                                                    device_manager);

        this->framebuffer = framebufferUtil::createBasicFramebuffer(*this->color_image,
                                                                    *this->depth_image,
                                                                    render_pass_registry,
                                                                    device_manager->getDevice());
    }

    bool BasicFramebuffer::hasColorImage() const {
        return true;
    }

    const IImage& BasicFramebuffer::getColorImage() const {
        return *this->color_image;
    }

    IImage& BasicFramebuffer::getColorImage() {
        return *this->color_image;
    }

    bool BasicFramebuffer::hasDepthImage() const {
        return true;
    }

    const IImage& BasicFramebuffer::getDepthImage() const {
        return *this->depth_image;
    }

    IImage& BasicFramebuffer::getDepthImage() {
        return *this->depth_image;
    }

    const vk::Framebuffer& BasicFramebuffer::getFramebuffer() const {
        return this->framebuffer;
    }

    BasicFramebuffer::~BasicFramebuffer() {
        this->device_manager->getDevice().destroy(this->framebuffer);
    }
};
