#pragma once

#include "./IImage.hpp"

#include "../core/DeviceManager.hpp"

namespace wg::internal {

    // Special case of an image, where the backing image is part of the swapchain
    class SwapchainColorImage : public IImage {
        vk::Image image;
        vk::ImageView view;

        vk::Extent2D dimensions;
        vk::ImageLayout intended_layout;

        std::shared_ptr<const DeviceManager> device_manager;

    public:

        virtual const vk::Image getImage() const;
        virtual const vk::DeviceMemory getMemory() const;
        virtual const vk::ImageView getView() const;
        virtual vk::Extent2D getDimensions() const;

        virtual const vk::ImageAspectFlagBits getDefaultAspect() const;

        virtual const vk::ImageLayout getIntendedLayout() const;

        SwapchainColorImage(const vk::Extent2D& dimensions,
                            const vk::Image& image,
                            std::shared_ptr<const DeviceManager> device_manager);

        ~SwapchainColorImage();
    };
};
