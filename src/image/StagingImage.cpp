#include "./StagingImage.hpp"

#include "./copyImage.hpp"
#include "./imageUtil.hpp"

#include "../memory/memoryUtil.hpp"
#include "../sync/fenceUtil.hpp"
#include "../util/log.hpp"

#include <flawed_assert.hpp>

namespace wg::internal {
    StagingImage::StagingImage(const vk::Extent2D& dimensions,
                               std::shared_ptr<DeviceManager> device_manager,
                               std::shared_ptr<QueueManager> queue_manager,
                               std::shared_ptr<CommandManager> command_manager)
        : staging_image(dimensions,
                        BasicImageSettings::createHostAccessibleColorImageSettings(),
                        device_manager), device_manager(device_manager),
          command_manager(command_manager)
    {
        this->command = this->command_manager->createGraphicsCommands(1)[0];
        this->graphics_queue = queue_manager->getGraphicsQueue();

        imageUtil::initializeLayout(this->staging_image,
                                    vk::ImageLayout::eGeneral,
                                    this->command,
                                    this->graphics_queue,
                                    this->device_manager->getDevice());
    }


    void StagingImage::set(const std::span<uint32_t>& src,
                           IImage& dst,
                           uint32_t src_byte_stride = 0) {
        if (src_byte_stride == 0) {
            src_byte_stride = dst.getDimensions().width * sizeof(uint32_t);
        }

        fl_assert_le(dst.getDimensions().width * sizeof(uint32_t), src_byte_stride);
        fl_assert_eq(dst.getDimensions().height, src.size() / src_byte_stride);

        uint32_t required_size_multiple = this->device_manager->getDeviceProperties().limits.nonCoherentAtomSize;

        vk::MappedMemoryRange range =
            memoryUtil::getMappedMemoryRangeForCopy(
                src_byte_stride * dst.getDimensions().height,
                0,
                required_size_multiple,
                dst.getDimensions().width * dst.getDimensions().height * sizeof(uint32_t), // Guess allocation size
                this->staging_image.getMemory());

        void* staging_dst = memoryUtil::mapMemory<void>(this->staging_image.getMemory(),
                                                        this->device_manager->getDevice());

        memcpy(staging_dst, src.data(), dst.getDimensions().height * src_byte_stride);

        this->device_manager->getDevice().flushMappedMemoryRanges({range});

        memoryUtil::unmapMemory(this->staging_image.getMemory(),
                                this->device_manager->getDevice());

        CopyImageAuxillaryData aux_data;
        copyImage::recordCopyImage(this->staging_image,
                                   dst,
                                   aux_data,
                                   this->command,
                                   this->device_manager->getDevice(),
                                   this->graphics_queue);

        vk::SubmitInfo si;
        si.setCommandBufferCount(1)
            .setPCommandBuffers(&this->command.buffer);

        fenceUtil::resetFence(this->command.fence, this->device_manager->getDevice());
        _wassert_result(this->graphics_queue.submit(1, &si, this->command.fence),
                        "submitting staging image copy command");

        fenceUtil::awaitFence(this->command.fence, this->device_manager->getDevice());
    }

    StagingImage::~StagingImage() {
        this->command_manager->destroyGraphicsCommands({this->command});
    }
};
