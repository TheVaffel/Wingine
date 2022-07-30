#pragma once

#include <vulkan/vulkan.hpp>

#include "./IImage.hpp"
#include "../DeviceManager.hpp"

namespace wg::internal {

    // Simple general-purpose image
    class BasicImage : public virtual IImage {
        vk::Image image;
        vk::DeviceMemory memory;
        vk::ImageView view;
        vk::Extent2D dimensions;

        vk::ImageLayout current_layout;
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

        virtual const vk::ImageLayout getCurrentLayout() const;
        virtual const vk::ImageLayout getIntendedLayout() const;

        // Improvement: Make private, change through friendly image mutator class?
        virtual void setCurrentLayout(const vk::ImageLayout& layout);

        BasicImage(const vk::Extent2D& dimensions,
                   const vk::ImageAspectFlagBits& aspect,
                   const vk::ImageLayout& intended_layout,
                   const vk::ImageUsageFlags& usage,
                   std::shared_ptr<const DeviceManager> device_manager);

        static std::unique_ptr<BasicImage>
        createFramebufferColorImage(const vk::Extent2D& dimensions,
                                    std::shared_ptr<const DeviceManager> device_manager);

        static std::unique_ptr<BasicImage>
        createFramebufferTextureColorImage(const vk::Extent2D& dimensions,
                                           std::shared_ptr<const DeviceManager> device_manager);

        static std::unique_ptr<BasicImage>
        createFramebufferDepthImage(const vk::Extent2D& dimensions,
                                    std::shared_ptr<const DeviceManager> device_manager);

        static std::unique_ptr<BasicImage>
        createFramebufferTextureDepthImage(const vk::Extent2D& dimensions,
                                           std::shared_ptr<const DeviceManager> device_manager);

        static std::unique_ptr<BasicImage>
        createHostAccessibleColorImage(const vk::Extent2D& dimensions,
                                       std::shared_ptr<const DeviceManager> device_manager);
        virtual ~BasicImage();
    };

};
