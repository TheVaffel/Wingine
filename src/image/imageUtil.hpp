#pragma once

#include <vulkan/vulkan.hpp>

#include "./CopyImageAuxillaryData.hpp"
#include "./IImage.hpp"

namespace wg::internal::imageUtil {

    const vk::Format DEFAULT_FRAMEBUFFER_COLOR_IMAGE_FORMAT = vk::Format::eB8G8R8A8Unorm;
    const vk::Format DEFAULT_FRAMEBUFFER_DEPTH_IMAGE_FORMAT = vk::Format::eD32Sfloat;

    /*
     * Image
     */

    vk::Image createSimpleImage(const vk::Extent2D& dimensions,
                                const vk::ImageUsageFlags& usage,
                                const vk::Format& format,
                                const vk::Device& device);

    vk::Image createFramebufferColorImage(const vk::Extent2D& dimensions,
                                          const vk::Format& format,
                                          const vk::Device& device);

    vk::Image createFramebufferTextureColorImage(const vk::Extent2D& dimensions,
                                                 const vk::Format& format,
                                                 const vk::Device& device);

    vk::Image createFramebufferDepthImage(const vk::Extent2D& dimensions,
                                          const vk::Format& format,
                                          const vk::Device& device);

    vk::Image createFramebufferTextureDepthImage(const vk::Extent2D& dimensions,
                                                 const vk::Format& format,
                                                 const vk::Device& device);

    vk::Image createHostAccessibleColorImage(const vk::Extent2D& dimensions,
                                             const vk::Format& format,
                                             const vk::Device& device);

    /*
     * ImageView
     */

    vk::ImageView createColorImageView(const vk::Image& image,
                                       const vk::Format& format,
                                       const vk::Device& device);

    vk::ImageView createDepthImageView(const vk::Image& image,
                                       const vk::Format& format,
                                       const vk::Device& device);

    /*
     * Recording
     */

    void recordSetLayout(CommandLayoutTransitionData& data,
                         const vk::CommandBuffer& command_buffer,
                         const vk::ImageLayout& old_layout,
                         const vk::ImageLayout& new_layout,
                         IImage& image);
};
