#include <vulkan/vulkan.hpp>

#include "./IImage.hpp"

#include "../DeviceManager.hpp"

namespace wg::internal {

    // Special case of an image, where the backing image is part of the swapchain
    class SwapchainColorImage : public IImage {
        vk::Image image;
        vk::DeviceMemory memory;
        vk::ImageView view;

        vk::Extent2D dimensions;
        vk::ImageLayout current_layout;

        std::shared_ptr<const DeviceManager> device_manager;

        SwapchainColorImage(const vk::Extent2D& dimensions,
                            std::shared_ptr<const DeviceManager> device_manager);

    public:

        virtual const vk::Image getImage() const;
        virtual const vk::DeviceMemory getMemory() const;
        virtual const vk::ImageView getView() const;
        virtual const vk::Extent2D getDimensions() const;

        virtual const vk::ImageLayout getCurrentLayout() const;
        virtual void setCurrentLayout(const vk::ImageLayout& layout);

        static std::unique_ptr<SwapchainColorImage>
        createFramebufferColorImageFromSwapchainImage(const vk::Image& image,
                                                      const vk::Extent2D& dimensions,
                                                      std::shared_ptr<const DeviceManager> device_manager);

        ~SwapchainColorImage();
    };
};
