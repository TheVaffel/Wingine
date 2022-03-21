#include "./SwapchainColorImage.hpp"

#include "./imageUtil.hpp"

namespace wg::internal {

    SwapchainColorImage::SwapchainColorImage(const vk::Extent2D& dimensions,
                                             std::shared_ptr<const DeviceManager> device_manager)
        : dimensions(dimensions),
          current_layout(vk::ImageLayout::eUndefined),
          device_manager(device_manager) {

    }


    std::unique_ptr<SwapchainColorImage>
    SwapchainColorImage::createFramebufferColorImageFromSwapchainImage(
        const vk::Image& image,
        const vk::Extent2D& dimensions,
        std::shared_ptr<const DeviceManager> device_manager
        ) {

        std::unique_ptr<SwapchainColorImage> im =
            std::unique_ptr<SwapchainColorImage>(new SwapchainColorImage(dimensions, device_manager));

        im->image = image;
        im->view = imageUtil::createColorImageView(im->image,
                                                   imageUtil::DEFAULT_FRAMEBUFFER_COLOR_IMAGE_FORMAT,
                                                   device_manager->getDevice());

        return im;
    }


    const vk::Image SwapchainColorImage::getImage() const {
        return this->image;
    }

    const vk::DeviceMemory SwapchainColorImage::getMemory() const {
        throw std::runtime_error("Cannot get memory of swapchain image");
    }

    const vk::ImageView SwapchainColorImage::getView() const {
        return this->view;
    }

    const vk::Extent2D SwapchainColorImage::getDimensions() const {
        return this->dimensions;
    }

    const vk::ImageLayout SwapchainColorImage::getCurrentLayout() const {
        return this->current_layout;
    }

    void SwapchainColorImage::setCurrentLayout(const vk::ImageLayout& layout) {
        this->current_layout = layout;
    }

    SwapchainColorImage::~SwapchainColorImage() {
        this->device_manager->getDevice().destroy(this->view);
    }
};
