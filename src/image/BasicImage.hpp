#pragma once

#include <vulkan/vulkan.hpp>

#include "./IImage.hpp"
#include "../DeviceManager.hpp"

namespace wg::internal {

    // Simple general-purpose image
    class BasicImage : public IImage {
        vk::Image image;
        vk::DeviceMemory memory;
        vk::ImageView view;
        vk::Extent2D dimensions;
        vk::ImageLayout current_layout;
        vk::ImageAspectFlagBits aspect;

        std::shared_ptr<const DeviceManager> device_manager;

        BasicImage(const vk::Extent2D& dimensions,
                   const vk::ImageAspectFlagBits& aspect,
                   std::shared_ptr<const DeviceManager> device_manager);

    public:

        virtual const vk::Image getImage() const;
        virtual const vk::DeviceMemory getMemory() const;
        virtual const vk::ImageView getView() const;
        virtual const vk::Extent2D getDimensions() const;

        virtual const vk::ImageAspectFlagBits getDefaultAspect() const;

        virtual const vk::ImageLayout getCurrentLayout() const;

        // Improvement: Make private, change through friendly image mutator class?
        virtual void setCurrentLayout(const vk::ImageLayout& layout);

        static std::unique_ptr<BasicImage>
        createFramebufferColorImage(const vk::Extent2D& dimensions,
                                    std::shared_ptr<const DeviceManager> device_manager);

        static std::unique_ptr<BasicImage>
        createFramebufferDepthImage(const vk::Extent2D& dimensions,
                                    std::shared_ptr<const DeviceManager> device_manager);

        ~BasicImage();
    };

};
