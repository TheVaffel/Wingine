#include "./SwapchainFramebuffer.hpp"

#include "./framebufferUtil.hpp"

#include "../image/SwapchainColorImage.hpp"
#include "../image/BasicImage.hpp"

namespace wg::internal {
    SwapchainFramebuffer::SwapchainFramebuffer(const vk::Image& image,
                                               std::shared_ptr<const SwapchainManager> swapchain_manager,
                                               std::shared_ptr<const DeviceManager> device_manager,
                                               CompatibleRenderPassRegistry& render_pass_registry)
        : swapchain_manager(swapchain_manager),
          color_image(swapchain_manager->getDimensions(),
                      image,
                      device_manager),
          depth_image(swapchain_manager->getDimensions(),
                      BasicImageSettings::createFramebufferDepthImageSettings(),
                      device_manager),
          device_manager(device_manager) {

        this->framebuffer = framebufferUtil::createBasicFramebuffer(this->color_image,
                                                                    this->depth_image,
                                                                    render_pass_registry,
                                                                    device_manager->getDevice());

    }

    std::unique_ptr<IFramebuffer> SwapchainFramebuffer::createSwapchainFramebuffer(
        const vk::Image& image,
        std::shared_ptr<const SwapchainManager> swapchain_manager,
        std::shared_ptr<const DeviceManager> device_manager,
        CompatibleRenderPassRegistry& render_pass_registry
        ) {

        return std::unique_ptr<IFramebuffer>(
            new SwapchainFramebuffer(
                image,
                swapchain_manager,
                device_manager,
                render_pass_registry));
    }

    vk::Extent2D SwapchainFramebuffer::getDimensions() const {
        return this->color_image.getDimensions();
    }

    const vk::Framebuffer& SwapchainFramebuffer::getFramebuffer() const {
        return this->framebuffer;
    }

    bool SwapchainFramebuffer::hasColorImage() const {
        return true;
    }

    const IImage& SwapchainFramebuffer::getColorImage() const {
        return this->color_image;
    }

    bool SwapchainFramebuffer::hasDepthImage() const {
        return true;
    }

    const IImage& SwapchainFramebuffer::getDepthImage() const {
        return this->depth_image;
    }

    SwapchainFramebuffer::~SwapchainFramebuffer() {
        this->device_manager->getDevice().destroy(this->framebuffer);
    }
};
