#include "./SwapchainManager.hpp"

#include "./image/imageUtil.hpp"
#include "./constants.hpp"

namespace wg::internal {

    namespace {
        vk::SurfaceFormatKHR getSurfaceFormat(const vk::PhysicalDevice& phDev,
                                              const vk::SurfaceKHR& surface) {
            std::vector<vk::SurfaceFormatKHR> surfaceFormats = phDev.getSurfaceFormatsKHR(surface);

            if(surfaceFormats.size() == 1 &&
               surfaceFormats[0].format == vk::Format::eUndefined) {
                return imageUtil::DEFAULT_FRAMEBUFFER_COLOR_IMAGE_FORMAT;
            } else {
                return surfaceFormats[0];
            }
        }

        vk::Extent2D getSurfaceDimensions(const vk::Extent2D& preferred_dimensions,
                                          const vk::SurfaceCapabilitiesKHR& caps) {

            vk::Extent2D swapchainExtent;
            if(caps.currentExtent.width == 0xFFFFFFFF) {
                swapchainExtent.width =
                    std::min(caps.maxImageExtent.width,
                             std::max(caps.minImageExtent.width,
                                      preferred_dimensions.width));
                swapchainExtent.height =
                    std::min(caps.maxImageExtent.height,
                             std::max(caps.minImageExtent.height,
                                      preferred_dimensions.height));

            } else {
                swapchainExtent = caps.currentExtent;
            }

            return swapchainExtent;
        }

        uint32_t getNumSwapchainImages(const vk::SurfaceCapabilitiesKHR& caps) {

            uint32_t numSwaps =
                std::max(caps.minImageCount, (uint32_t) constants::preferred_swapchain_image_count);

            if(caps.maxImageCount != 0) {
                numSwaps =
                    std::min(caps.maxImageCount, numSwaps);
            }

            return numSwaps;
        }

        vk::SurfaceTransformFlagBitsKHR getPreTransform(const vk::SurfaceCapabilitiesKHR& caps) {
            if((caps.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity) ==
               vk::SurfaceTransformFlagBitsKHR::eIdentity) {
                return vk::SurfaceTransformFlagBitsKHR::eIdentity;
            } else {
                return caps.currentTransform;
            }
        }

        vk::CompositeAlphaFlagBitsKHR getCompositeAlpha(const vk::SurfaceCapabilitiesKHR& caps) {

            vk::CompositeAlphaFlagBitsKHR alphaFlags[4] = {
                vk::CompositeAlphaFlagBitsKHR::eOpaque,
                vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
                vk::CompositeAlphaFlagBitsKHR::ePostMultiplied,
                vk::CompositeAlphaFlagBitsKHR::eInherit
            };

            for(vk::CompositeAlphaFlagBitsKHR bit : alphaFlags) {
                if((caps.supportedCompositeAlpha & bit) == bit) {
                    return bit;
                }
            }

            throw std::runtime_error("No meaningful CompositeAlphaFlagBit is supported by surface");
        }

        vk::PresentModeKHR getPresentMode(const vk::PhysicalDevice& physical_device,
                                          const vk::SurfaceKHR& surface) {

            std::vector<vk::PresentModeKHR> present_modes =
                physical_device.getSurfacePresentModesKHR(surface);

            for(vk::PresentModeKHR mode : present_modes) {
                if(mode == vk::PresentModeKHR::eMailbox) {
                    return vk::PresentModeKHR::eMailbox;
                }
            }

            return vk::PresentModeKHR::eFifo;
        }

        vk::SwapchainKHR createSwapchain(const DeviceManager& device_manager,
                                         const QueueManager& queue_manager,
                                         const vk::SurfaceKHR& surface,
                                         const vk::Extent2D& swapchain_extent,
                                         const vk::SurfaceCapabilitiesKHR& caps) {

            auto [ format, colorSpace ] = getSurfaceFormat(device_manager.getPhysicalDevice(),
                                                                   surface);
            vk::Format surface_format = format;

            uint32_t num_swapchain_images = getNumSwapchainImages(caps);
            vk::SurfaceTransformFlagBitsKHR preTransform = getPreTransform(caps);

            vk::CompositeAlphaFlagBitsKHR compositeAlpha = getCompositeAlpha(caps);
            vk::PresentModeKHR swapchain_present_mode = getPresentMode(device_manager.getPhysicalDevice(),
                                                                       surface);

            vk::SwapchainCreateInfoKHR sci;
            sci.setSurface(surface)
                .setMinImageCount(num_swapchain_images)
                .setImageFormat(surface_format)
                .setImageExtent(swapchain_extent)
                .setPreTransform(preTransform)
                .setCompositeAlpha(compositeAlpha)
                .setImageArrayLayers(1)
                .setPresentMode(swapchain_present_mode)
                .setOldSwapchain(nullptr)
                .setClipped(true)
                .setImageColorSpace(colorSpace)
                //      | vk::ImageUsageFlagBits::eTransferSrc)
                .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
                .setImageSharingMode(vk::SharingMode::eExclusive)
                .setQueueFamilyIndexCount(0)
                .setPQueueFamilyIndices(nullptr);

            if(queue_manager.getGraphicsQueueIndex() != queue_manager.getPresentQueueIndex()) {

                uint32_t queue_indices[2] = {
                    (uint32_t) queue_manager.getGraphicsQueueIndex(),
                    (uint32_t) queue_manager.getPresentQueueIndex()
                };

                sci.setImageSharingMode(vk::SharingMode::eConcurrent)
                    .setQueueFamilyIndexCount(2)
                    .setPQueueFamilyIndices(queue_indices);

            }

            return device_manager.getDevice().createSwapchainKHR(sci);
        }
    };

    SwapchainManager::SwapchainManager(const vk::Extent2D& preferred_dimensions,
                                       const vk::SurfaceKHR& surface,
                                       std::shared_ptr<const DeviceManager> device_manager,
                                       const QueueManager& queue_manager)
        : device_manager(device_manager) {

        const vk::PhysicalDevice& physical_device = device_manager->getPhysicalDevice();
        const vk::Device& device = device_manager->getDevice();

        vk::SurfaceCapabilitiesKHR caps =
            physical_device.getSurfaceCapabilitiesKHR(surface);

        vk::Extent2D swapchain_extent = getSurfaceDimensions(preferred_dimensions, caps);
        this->window_dimensions = swapchain_extent;

        this->swapchain = createSwapchain(*device_manager,
                                          queue_manager,
                                          surface,
                                          swapchain_extent,
                                          caps);

        this->swapchain_images = device.getSwapchainImagesKHR(swapchain);

    }

    const vk::SwapchainKHR& SwapchainManager::getSwapchain() const {
        return this->swapchain;
    }

    const vk::Extent2D& SwapchainManager::getDimensions() const {
        return this->window_dimensions;
    }

    const std::vector<vk::Image>& SwapchainManager::getImages() const {
        return this->swapchain_images;
    }

    uint32_t SwapchainManager::getNumImages() const {
        return this->swapchain_images.size();
    }


    SwapchainManager::~SwapchainManager() {
        const vk::Device& device = this->device_manager->getDevice();

        device.destroy(this->swapchain);
    }


    /*
     * Static methods
     */

    uint32_t SwapchainManager::getNumFramebuffers(const vk::SurfaceKHR& surface,
                                                  std::shared_ptr<const DeviceManager> device_manager) {

        vk::SurfaceCapabilitiesKHR caps =
            device_manager->getPhysicalDevice().getSurfaceCapabilitiesKHR(surface);

        return getNumSwapchainImages(caps);
    }
};
