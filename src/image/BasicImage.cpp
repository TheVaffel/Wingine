#include "./BasicImage.hpp"

#include "imageUtil.hpp"
#include "../memory/memoryUtil.hpp"

namespace wg::internal {

    BasicImage::BasicImage(const vk::Extent2D& dimensions,
                           const BasicImageSetup& settings,
                           std::shared_ptr<const DeviceManager> device_manager)
        : dimensions(dimensions),
          intended_layout(settings.intended_layout),
          aspect(settings.aspect),
          device_manager(device_manager) {
        this->image = imageUtil::createSimpleImage(dimensions,
                                                   settings.usage,
                                                   settings.format,
                                                   settings.tiling,
                                                   device_manager->getDevice());

        this->memory =
            memoryUtil::createAndBindMemoryForImage(this->image,
                                                    settings.memory_properties,
                                                    device_manager->getDevice(),
                                                    device_manager->getDeviceMemoryProperties());

        if (settings.with_view) {
            this->view = imageUtil::createImageView(this->image,
                                                    settings.format,
                                                    settings.aspect,
                                                    device_manager->getDevice());
        }
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

    vk::Extent2D BasicImage::getDimensions() const {
        return this->dimensions;
    }

    const vk::ImageAspectFlagBits BasicImage::getDefaultAspect() const {
        return this->aspect;
    }

    const vk::ImageLayout BasicImage::getIntendedLayout() const {
        return this->intended_layout;
    }

    BasicImage::~BasicImage() {
        this->device_manager->getDevice().destroy(this->view);
        this->device_manager->getDevice().freeMemory(this->memory);
        this->device_manager->getDevice().destroy(this->image);
    }
};
