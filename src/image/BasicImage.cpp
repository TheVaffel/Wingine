#include "./BasicImage.hpp"

#include "imageUtil.hpp"
#include "../memory/memoryUtil.hpp"

namespace wg::internal {

    BasicImage::BasicImage(const vk::Extent2D& dimensions,
                           const vk::ImageAspectFlagBits& aspect,
                           const vk::ImageLayout& intended_layout,
                           std::shared_ptr<const DeviceManager> device_manager)
        : dimensions(dimensions),
          current_layout(vk::ImageLayout::eUndefined),
          intended_layout(intended_layout),
          aspect(aspect),
          device_manager(device_manager)
    { }

    BasicImage::BasicImage(const vk::Extent2D& dimensions,
                           const vk::ImageAspectFlagBits& aspect,
                           const vk::ImageLayout& intended_layout,
                           const vk::ImageUsageFlags& usage,
                           std::shared_ptr<const DeviceManager> device_manager)
        : dimensions(dimensions),
          current_layout(vk::ImageLayout::eUndefined),
          intended_layout(intended_layout),
          aspect(aspect),
          device_manager(device_manager) {

        bool isDepth = aspect == vk::ImageAspectFlagBits::eDepth;
        const vk::Format format = isDepth ?
            imageUtil::DEFAULT_FRAMEBUFFER_DEPTH_IMAGE_FORMAT :
            imageUtil::DEFAULT_FRAMEBUFFER_COLOR_IMAGE_FORMAT;

        this->image = imageUtil::createSimpleImage(dimensions,
                                                  usage,
                                                  format,
                                                  device_manager->getDevice());

        this->memory =
            memoryUtil::createAndBindMemoryForImage(this->image,
                                                    device_manager->getDevice(),
                                                    device_manager->getDeviceMemoryProperties());

        if (isDepth) {
            this->view = imageUtil::createDepthImageView(this->image,
                                                         format,
                                                         device_manager->getDevice());
        } else {
            this->view = imageUtil::createColorImageView(this->image,
                                                         format,
                                                         device_manager->getDevice());
        }
    }

    std::unique_ptr<BasicImage>
    BasicImage::createFramebufferColorImage(const vk::Extent2D& dimensions,
                                            std::shared_ptr<const DeviceManager> device_manager) {

        std::unique_ptr<BasicImage> im = std::unique_ptr<BasicImage>(
            new BasicImage(dimensions,
                           vk::ImageAspectFlagBits::eColor,
                           vk::ImageLayout::ePresentSrcKHR,
                           device_manager));

        im->image = imageUtil::createFramebufferColorImage(dimensions,
                                                           imageUtil::DEFAULT_FRAMEBUFFER_COLOR_IMAGE_FORMAT,
                                                           device_manager->getDevice());

        im->memory = memoryUtil::createAndBindMemoryForImage(im->image,
                                                             device_manager->getDevice(),
                                                             device_manager->getDeviceMemoryProperties());

        im->view = imageUtil::createColorImageView(im->image,
                                                   imageUtil::DEFAULT_FRAMEBUFFER_COLOR_IMAGE_FORMAT,
                                                   device_manager->getDevice());

        return im;
    }

    std::unique_ptr<BasicImage>
    BasicImage::createFramebufferDepthImage(const vk::Extent2D& dimensions,
                                            std::shared_ptr<const DeviceManager> device_manager) {

        std::unique_ptr<BasicImage> im = std::unique_ptr<BasicImage>(
            new BasicImage(dimensions,
                           vk::ImageAspectFlagBits::eDepth,
                           vk::ImageLayout::eDepthStencilAttachmentOptimal,
                           device_manager));

        im->image = imageUtil::createFramebufferDepthImage(dimensions,
                                                           imageUtil::DEFAULT_FRAMEBUFFER_DEPTH_IMAGE_FORMAT,
                                                           device_manager->getDevice());

        im->memory = memoryUtil::createAndBindMemoryForImage(im->image,
                                                             device_manager->getDevice(),
                                                             device_manager->getDeviceMemoryProperties());

        im->view = imageUtil::createDepthImageView(im->image,
                                                   imageUtil::DEFAULT_FRAMEBUFFER_DEPTH_IMAGE_FORMAT,
                                                   device_manager->getDevice());

        return im;
    }

    std::unique_ptr<BasicImage>
    BasicImage::createHostAccessibleColorImage(const vk::Extent2D& dimensions,
                                               std::shared_ptr<const DeviceManager> device_manager) {

        vk::Format host_accessible_color_image_format = vk::Format::eB8G8R8A8Sint;
        std::unique_ptr<BasicImage> im = std::unique_ptr<BasicImage>(
            new BasicImage(dimensions,
                           vk::ImageAspectFlagBits::eColor,
                           vk::ImageLayout::eGeneral,
                           device_manager));

        im->image = imageUtil::createHostAccessibleColorImage(dimensions,
                                                              host_accessible_color_image_format,
                                                              device_manager->getDevice());

        im->memory =
            memoryUtil::createAndBindHostAccessibleMemoryForImage(im->image,
                                                                  device_manager->getDevice(),
                                                                  device_manager->getDeviceMemoryProperties());

        im->view = imageUtil::createColorImageView(im->image,
                                                   host_accessible_color_image_format,
                                                   device_manager->getDevice());

        return im;
    }

    const vk::Image BasicImage::getImage() const {
        return this->image;
    }

    const vk::DeviceMemory BasicImage::getMemory() const {
        return this->memory;
    }

    const vk::ImageView BasicImage::getView() const {
        return this->view;
    }

    const vk::Extent2D BasicImage::getDimensions() const {
        return this->dimensions;
    }

    const vk::ImageAspectFlagBits BasicImage::getDefaultAspect() const {
        return this->aspect;
    }

    const vk::ImageLayout BasicImage::getCurrentLayout() const {
        return this->current_layout;
    }

    const vk::ImageLayout BasicImage::getIntendedLayout() const {
        return this->intended_layout;
    }

    void BasicImage::setCurrentLayout(const vk::ImageLayout& layout) {
        this->current_layout = layout;
    }

    BasicImage::~BasicImage() {
        this->device_manager->getDevice().destroy(this->view);
        this->device_manager->getDevice().freeMemory(this->memory);
        this->device_manager->getDevice().destroy(this->image);
    }
};
