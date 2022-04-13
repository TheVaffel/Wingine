#include "./DepthOnlyFramebuffer.hpp"
#include "./IFramebuffer.hpp"

#include "./framebufferUtil.hpp"

namespace wg::internal {

    DepthOnlyFramebuffer::DepthOnlyFramebuffer(const vk::Extent2D& dimensions,
                                               std::shared_ptr<const DeviceManager> device_manager,
                                               CompatibleRenderPassRegistry& render_pass_registry)
        : device_manager(device_manager) {
        this->depth_image = BasicImage::createFramebufferDepthImage(dimensions,
                                                                    device_manager);

        this->framebuffer = framebufferUtil::createDepthOnlyFramebuffer(*this->depth_image,
                                                                        render_pass_registry,
                                                                        device_manager->getDevice());
    }

    vk::Extent2D DepthOnlyFramebuffer::getDimensions() const {
        return this->depth_image->getDimensions();
    }

    bool DepthOnlyFramebuffer::hasColorImage() const {
        return false;
    }

    const IImage& DepthOnlyFramebuffer::getColorImage() const {
        throw std::runtime_error("[DepthOnlyFramebuffer] Does not have color image");
    }

    IImage& DepthOnlyFramebuffer::getColorImage() {
        throw std::runtime_error("[DepthOnlyFramebuffer] Does not have color image");
    }

    bool DepthOnlyFramebuffer::hasDepthImage() const {
        return true;
    }

    const IImage& DepthOnlyFramebuffer::getDepthImage() const {
        return *this->depth_image;
    }

    IImage& DepthOnlyFramebuffer::getDepthImage() {
        return *this->depth_image;
    }

    const vk::Framebuffer& DepthOnlyFramebuffer::getFramebuffer() const {
        return this->framebuffer;
    }

    DepthOnlyFramebuffer::~DepthOnlyFramebuffer() {
        this->device_manager->getDevice().destroy(this->framebuffer);
    }
};
