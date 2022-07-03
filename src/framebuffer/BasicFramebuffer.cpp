#include "./BasicFramebuffer.hpp"

#include "../image/BasicImage.hpp"
#include "./framebufferUtil.hpp"

namespace wg::internal {

BasicFramebuffer::BasicFramebuffer(const vk::Extent2D& dimensions,
                                       const BasicFramebufferSetup& setup,
                                       std::shared_ptr<const DeviceManager> device_manager,
                                       CompatibleRenderPassRegistry& render_pass_registry)
        : device_manager(device_manager) {

        this->initializeColorImage(dimensions, setup);
        this->initializeDepthImage(dimensions, setup);

        if (setup.getDepthOnly()) {
            this->framebuffer = framebufferUtil::createDepthOnlyFramebuffer(*this->depth_image,
                                                                            render_pass_registry,
                                                                            device_manager->getDevice());
        } else {
            this->framebuffer = framebufferUtil::createBasicFramebuffer(*this->color_image,
                                                                        *this->depth_image,
                                                                        render_pass_registry,
                                                                        device_manager->getDevice());
        }
    }

    void BasicFramebuffer::initializeColorImage(const vk::Extent2D& dimensions,
                                                const BasicFramebufferSetup& setup) {
        if (!setup.getDepthOnly()) {
            if (setup.getSamplable()) {
                this->color_image = BasicImage::createFramebufferTextureColorImage(dimensions,
                                                                                   this->device_manager);
            } else {
                this->color_image = BasicImage::createFramebufferColorImage(dimensions,
                                                                            this->device_manager);
            }
        }
    }

    void BasicFramebuffer::initializeDepthImage(const vk::Extent2D& dimensions,
                                                const BasicFramebufferSetup& setup) {
        if (setup.getSamplable()) {
            this->depth_image = BasicImage::createFramebufferTextureDepthImage(dimensions,
                                                                               this->device_manager);
        } else {
            this->depth_image = BasicImage::createFramebufferDepthImage(dimensions,
                                                                        this->device_manager);
        }
    }

    vk::Extent2D BasicFramebuffer::getDimensions() const {
        return this->hasColorImage() ? this->color_image->getDimensions() : this->depth_image->getDimensions();
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
