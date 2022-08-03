#pragma once

#include "./IImage.hpp"
#include "../DeviceManager.hpp"

#include "./BasicImageSetup.hpp"

namespace wg::internal {

    // Simple general-purpose image
    class BasicImage : public virtual IImage {
        vk::Image image;
        vk::DeviceMemory memory;
        vk::ImageView view;
        vk::Extent2D dimensions;

        vk::ImageLayout intended_layout;

        vk::ImageAspectFlagBits aspect;

        BasicImage(const vk::Extent2D& dimensions,
                   const vk::ImageAspectFlagBits& aspect,
                   const vk::ImageLayout& intended_layout,
                   std::shared_ptr<const DeviceManager> device_manager);

    protected:

        std::shared_ptr<const DeviceManager> device_manager;

    public:

        virtual const vk::Image getImage() const;
        virtual const vk::DeviceMemory getMemory() const;
        virtual const vk::ImageView getView() const;
        virtual vk::Extent2D getDimensions() const;

        virtual const vk::ImageAspectFlagBits getDefaultAspect() const;

        virtual const vk::ImageLayout getIntendedLayout() const;

        BasicImage(const vk::Extent2D& dimensions,
                   const BasicImageSetup& basic_image_settings,
                   std::shared_ptr<const DeviceManager> device_manager);
        virtual ~BasicImage();
    };

};
