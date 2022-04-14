#include "./copyImage.hpp"
#include "../CommandManager.hpp"
#include "../sync/fenceUtil.hpp"

#include <flawed_assert.hpp>

namespace wg::internal::copyImage {

    namespace {

        vk::ImageSubresourceLayers getSubresourceLayers(const IImage& image) {
            vk::ImageSubresourceLayers subresource_layers;
            subresource_layers
                .setAspectMask(image.getDefaultAspect())
                .setMipLevel(0)
                .setBaseArrayLayer(0)
                .setLayerCount(1);

            return subresource_layers;
        }

        vk::ImageSubresourceRange getSubresourceRange(const IImage& image) {

            vk::ImageSubresourceRange subresource_range;
            subresource_range.setAspectMask(image.getDefaultAspect())
                .setLevelCount(1)
                .setBaseArrayLayer(0)
                .setLayerCount(1);
            return subresource_range;
        }

        vk::ImageCopy createImageCopyInfo(const IImage& src,
                                          const IImage& dst) {
            vk::ImageCopy image_copy;
            vk::Offset3D offs;
            offs.setX(0).setY(0).setZ(0);

            vk::Extent3D ext_src;
            ext_src.setWidth(src.getDimensions().width).setHeight(src.getDimensions().height).setDepth(1);

            vk::ImageSubresourceLayers src_subresource = getSubresourceLayers(src),
                dst_subresource = getSubresourceLayers(dst);

            image_copy.setSrcSubresource(src_subresource)
                .setSrcOffset(offs)
                .setDstSubresource(dst_subresource)
                .setDstOffset(offs)
                .setExtent(ext_src);

            return image_copy;
        }

        void recordSetLayout(CommandLayoutTransitionData& data,
                             const vk::CommandBuffer& command_buffer,
                             const vk::ImageLayout& new_layout,
                             IImage& image) {

            vk::ImageSubresourceRange subresource_range = getSubresourceRange(image);

            data.image_memory_barrier.setOldLayout(image.getCurrentLayout())
                .setNewLayout(new_layout)
                .setImage(image.getImage())
                .setSubresourceRange(subresource_range)
                .setSrcAccessMask(vk::AccessFlagBits::eTransferRead)
                .setDstAccessMask(vk::AccessFlagBits::eTransferWrite);

            command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                           vk::PipelineStageFlagBits::eTransfer,
                                           {},
                                           0,
                                           nullptr,
                                           0,
                                           nullptr,
                                           1,
                                           &data.image_memory_barrier);
        }

        void recordImageCopy(const vk::CommandBuffer& command_buffer,
                             const IImage& src,
                             const IImage& dst) {

            vk::ImageCopy image_copy = createImageCopyInfo(src, dst);

            command_buffer
                .copyImage(src.getImage(),
                           vk::ImageLayout::eTransferSrcOptimal,
                           dst.getImage(),
                           vk::ImageLayout::eTransferDstOptimal,
                           1,
                           &image_copy);
        }

    };

    void recordCopyImage(IImage& src,
                         IImage& dst,
                         CopyImageAuxillaryData& auxillary_data,
                         const Command& command,
                         const vk::Device& device,
                         const vk::Queue& queue) {
        fl_assert_eq(src.getDimensions().width, dst.getDimensions().width);
        fl_assert_eq(src.getDimensions().height, dst.getDimensions().height);

        fenceUtil::awaitAndResetFence(command.fence, device);

        vk::CommandBufferBeginInfo begin_info;
        command.buffer.begin(begin_info);

        vk::ImageLayout original_src_layout = src.getCurrentLayout();
        vk::ImageLayout original_dst_layout = dst.getCurrentLayout();

        recordSetLayout(auxillary_data.src_initial_transition_data,
                        command.buffer,
                        vk::ImageLayout::eTransferSrcOptimal,
                        src);
        recordSetLayout(auxillary_data.dst_initial_transition_data,
                        command.buffer,
                        vk::ImageLayout::eTransferDstOptimal,
                        dst);

        recordImageCopy(command.buffer,
                        src,
                        dst);

        recordSetLayout(auxillary_data.src_final_transition_data,
                        command.buffer,
                        original_src_layout,
                        src);

        recordSetLayout(auxillary_data.dst_final_transition_data,
                        command.buffer,
                        original_dst_layout,
                        dst);

        command.buffer.end();
    }
};
