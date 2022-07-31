#include "./SwapchainColorImage.hpp"

#include "./imageUtil.hpp"

namespace wg::internal {

    SwapchainColorImage::SwapchainColorImage(const vk::Extent2D& dimensions,
                                             const vk::Image& image,
                                             std::shared_ptr<const DeviceManager> device_manager)
        :
          image(image),
          view(imageUtil::createColorImageView(image,
                                               imageUtil::DEFAULT_FRAMEBUFFER_COLOR_IMAGE_FORMAT,
                                               device_manager->getDevice())),
          dimensions(dimensions),
          intended_layout(vk::ImageLayout::ePresentSrcKHR),
          device_manager(device_manager) {

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

    vk::Extent2D SwapchainColorImage::getDimensions() const {
        return this->dimensions;
    }

    const vk::ImageAspectFlagBits SwapchainColorImage::getDefaultAspect() const {
        return vk::ImageAspectFlagBits::eColor;
    }

    const vk::ImageLayout SwapchainColorImage::getIntendedLayout() const {
        return this->intended_layout;
    }

    SwapchainColorImage::~SwapchainColorImage() {
        this->device_manager->getDevice().destroy(this->view);
    }
};
