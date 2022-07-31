#pragma once

#include <vulkan/vulkan.hpp>

namespace wg::internal {
    struct BasicImageSettings {
        /*
         * Initialized from user in constructor
         */
        vk::ImageAspectFlagBits aspect;
        vk::ImageLayout intended_layout;
        vk::ImageUsageFlags usage;

        /*
         * Initialized by inference in constructor
         */
        vk::Format format;

        /*
         * Not initialized
         */
        vk::ImageTiling tiling = vk::ImageTiling::eOptimal;
        vk::MemoryPropertyFlagBits memory_properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
        bool with_view = true;

        BasicImageSettings(vk::ImageAspectFlagBits aspect,
                           vk::ImageLayout intended_layout,
                           vk::ImageUsageFlags usage);

        static BasicImageSettings createFramebufferColorImageSettings();
        static BasicImageSettings createFramebufferDepthImageSettings();
        static BasicImageSettings createFramebufferTextureColorImageSettings();
        static BasicImageSettings createFramebufferTextureDepthImageSettings();
        static BasicImageSettings createHostAccessibleColorImageSettings();
        static BasicImageSettings createColorTextureImageSettings();
    };
};
