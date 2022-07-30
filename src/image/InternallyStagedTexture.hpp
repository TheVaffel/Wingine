#pragma once

#include "./BasicTexture.hpp"
#include "./StagingImage.hpp"

#include <span>

namespace wg::internal {
    class InternallyStagedTexture : public BasicTexture {
        StagingImage staging_image;

    public:
        InternallyStagedTexture(const vk::Extent2D& dimensions,
                                std::shared_ptr<DeviceManager> device_manager,
                                std::shared_ptr<QueueManager> queue_manager,
                                std::shared_ptr<CommandManager> command_manager);

        void set(const std::span<uint32_t>& src, uint32_t src_byte_stride);
    };
}
