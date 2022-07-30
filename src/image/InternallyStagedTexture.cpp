#include "./InternallyStagedTexture.hpp"

namespace wg::internal {

    InternallyStagedTexture::InternallyStagedTexture(const vk::Extent2D& dimensions,
                                                     std::shared_ptr<DeviceManager> device_manager,
                                                     std::shared_ptr<QueueManager> queue_manager,
                                                     std::shared_ptr<CommandManager> command_manager)
        : BasicTexture(dimensions, BasicTextureSetup(), device_manager),
          staging_image(dimensions, device_manager, queue_manager, command_manager) { }

    void InternallyStagedTexture::set(const std::span<uint32_t>& src, uint32_t src_byte_stride) {
        this->staging_image.set(src, *this, src_byte_stride);
    }
};
