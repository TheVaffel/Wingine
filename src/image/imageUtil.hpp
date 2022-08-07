#pragma once

#include <vulkan/vulkan.hpp>

#include "./CopyImageAuxillaryData.hpp"
#include "./IImage.hpp"
#include "../core/CommandManager.hpp"

namespace wg::internal::imageUtil {

    const vk::Format DEFAULT_FRAMEBUFFER_COLOR_IMAGE_FORMAT = vk::Format::eB8G8R8A8Unorm;
    const vk::Format DEFAULT_FRAMEBUFFER_DEPTH_IMAGE_FORMAT = vk::Format::eD32Sfloat;

    /*
     * Image
     */

    vk::Image createSimpleImage(const vk::Extent2D& dimensions,
                                const vk::ImageUsageFlags& usage,
                                const vk::Format& format,
                                const vk::ImageTiling& tiling,
                                const vk::Device& device);

    /*
     * ImageView
     */

    vk::ImageView createImageView(const vk::Image& image,
                                  const vk::Format& format,
                                  const vk::ImageAspectFlagBits& aspect,
                                  const vk::Device& device);

    vk::ImageView createColorImageView(const vk::Image& image,
                                       const vk::Format& format,
                                       const vk::Device& device);

    vk::ImageView createDepthImageView(const vk::Image& image,
                                       const vk::Format& format,
                                       const vk::Device& device);

    /*
     * Image layout
     */

    void initializeLayout(const IImage& image,
                          const vk::ImageLayout& layout,
                          const Command& command,
                          const vk::Queue& queue,
                          const vk::Device& device);

    void setLayout(const IImage& image,
                   const vk::ImageLayout& initial_layout,
                   const vk::ImageLayout& final_layout,
                   const Command& command,
                   const vk::Queue& queue,
                   const vk::Device& device);

    /*
     * Recording
     */

    void recordSetLayout(CommandLayoutTransitionData& data,
                         const vk::CommandBuffer& command_buffer,
                         const vk::ImageLayout& old_layout,
                         const vk::ImageLayout& new_layout,
                         const IImage& image);
};
