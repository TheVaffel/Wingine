#pragma once

#include <vulkan/vulkan.hpp>

#include "./DeviceManager.hpp"
#include "./QueueManager.hpp"
#include "./semaphore.hpp"

namespace wg::internal {
    class SwapchainManager {

        vk::SwapchainKHR swapchain;
        std::vector<vk::Image> swapchain_images;
        vk::Extent2D window_dimensions;

        std::shared_ptr<const DeviceManager> device_manager;

    public:

        SwapchainManager(const vk::Extent2D& preferred_dimensions,
                         const vk::SurfaceKHR& surface,
                         std::shared_ptr<const DeviceManager> device_manager,
                         const QueueManager& queue_manager);
        ~SwapchainManager();

        SwapchainManager(const SwapchainManager& swapchain_manager) = delete;

        const vk::SwapchainKHR& getSwapchain() const;
        const vk::Extent2D& getDimensions() const;

        const std::vector<vk::Image>& getImages() const;
        uint32_t getNumImages() const;

        static uint32_t getNumFramebuffers(const vk::SurfaceKHR& surface,
                                           std::shared_ptr<const DeviceManager> device_manager);
    };
};
