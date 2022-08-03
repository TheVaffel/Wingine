#pragma once

#include <vulkan/vulkan.hpp>

namespace wg::internal {
    struct BasicImageSetup {
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

        BasicImageSetup(vk::ImageAspectFlagBits aspect,
                           vk::ImageLayout intended_layout,
                           vk::ImageUsageFlags usage);

        static BasicImageSetup createFramebufferColorImageSetup();
        static BasicImageSetup createFramebufferDepthImageSetup();
        static BasicImageSetup createFramebufferTextureColorImageSetup();
        static BasicImageSetup createFramebufferTextureDepthImageSetup();
        static BasicImageSetup createHostAccessibleColorImageSetup();
        static BasicImageSetup createColorTextureImageSetup();
        static BasicImageSetup createStorageTextureSetup();
    };
};
