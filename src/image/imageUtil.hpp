#pragma once

#include <vulkan/vulkan.hpp>

namespace wg::internal::imageUtil {

    const vk::Format DEFAULT_FRAMEBUFFER_COLOR_IMAGE_FORMAT = vk::Format::eB8G8R8A8Unorm;
    const vk::Format DEFAULT_FRAMEBUFFER_DEPTH_IMAGE_FORMAT = vk::Format::eD32Sfloat;

    vk::Image createFramebufferColorImage(const vk::Extent2D& dimensions,
                                          const vk::Format& format,
                                          const vk::Device& device);

    vk::Image createFramebufferDepthImage(const vk::Extent2D& dimensions,
                                          const vk::Format& format,
                                          const vk::Device& device);


    vk::ImageView createColorImageView(const vk::Image& image,
                                       const vk::Format& format,
                                       const vk::Device& device);

    vk::ImageView createDepthImageView(const vk::Image& image,
                                       const vk::Format& format,
                                       const vk::Device& device);


};
