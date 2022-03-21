#pragma once

#include <vulkan/vulkan.hpp>

namespace wg::internal {

    // Simple interface for multipurpose images
    class IImage {
    public:
        virtual const vk::Image getImage() const = 0;
        virtual const vk::DeviceMemory getMemory() const = 0;
        virtual const vk::ImageView getView() const = 0;
        virtual const vk::Extent2D getDimensions() const = 0;

        virtual const vk::ImageLayout getCurrentLayout() const = 0;
        virtual void setCurrentLayout(const vk::ImageLayout& layout) = 0;

        virtual ~IImage();
    };
};
