#pragma once

#include "./BasicImage.hpp"

#include "../CommandManager.hpp"

#include <span>

namespace wg::internal {
    class StagingImage {
        std::unique_ptr<BasicImage> staging_image;

        Command command;
        vk::Queue graphics_queue;

        std::shared_ptr<DeviceManager> device_manager;
        std::shared_ptr<CommandManager> command_manager;

    public:
        StagingImage(const vk::Extent2D& dimensions,
                     std::shared_ptr<DeviceManager> device_manager,
                     std::shared_ptr<QueueManager> queue_manager,
                     std::shared_ptr<CommandManager> command_manager);

        void set(const std::span<uint32_t>& src, IImage& dst, uint32_t src_byte_stride);

        ~StagingImage();
    };
};
