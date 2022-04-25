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

        vk::BufferImageCopy createImageToBufferCopyInfo(const IImage& src,
                                                        const IBuffer& dst) {
            vk::BufferImageCopy buffer_image_copy;
            vk::ImageSubresourceLayers subresource = getSubresourceLayers(src);
            buffer_image_copy
                .setBufferOffset(0)
                .setImageSubresource(subresource)
                .setImageOffset({ 0, 0, 0 })
                .setImageExtent({ src.getDimensions().width,
                        src.getDimensions().height,
                        1 });

            return buffer_image_copy;
        }

        vk::AccessFlagBits getAccessMask(const vk::ImageLayout& layout) {
            switch(layout) {
            case vk::ImageLayout::eColorAttachmentOptimal:
            case vk::ImageLayout::ePresentSrcKHR:
                return vk::AccessFlagBits::eColorAttachmentWrite;
            case vk::ImageLayout::ePreinitialized:
                return vk::AccessFlagBits::eHostWrite;
            case vk::ImageLayout::eShaderReadOnlyOptimal:
                return vk::AccessFlagBits::eShaderRead;
            case vk::ImageLayout::eTransferDstOptimal:
                return vk::AccessFlagBits::eTransferWrite;
            case vk::ImageLayout::eTransferSrcOptimal:
                return vk::AccessFlagBits::eTransferRead;
            case vk::ImageLayout::eDepthStencilAttachmentOptimal:
                return vk::AccessFlagBits::eDepthStencilAttachmentWrite;
            default:
                throw std::runtime_error("[copyImage] No applicable access mask for layout " +
                                         vk::to_string(layout));
            }
        }

        vk::PipelineStageFlags getPipelineStageFromLayout(const vk::ImageLayout& layout) {
            switch (layout) {
            case vk::ImageLayout::eTransferDstOptimal:
                return vk::PipelineStageFlagBits::eTransfer;
            case vk::ImageLayout::eTransferSrcOptimal:
                return vk::PipelineStageFlagBits::eTransfer;
            case vk::ImageLayout::ePresentSrcKHR:
            case vk::ImageLayout::eColorAttachmentOptimal:
                return vk::PipelineStageFlagBits::eColorAttachmentOutput;
            case vk::ImageLayout::eShaderReadOnlyOptimal:
                return vk::PipelineStageFlagBits::eComputeShader |
                    vk::PipelineStageFlagBits::eFragmentShader;
            case vk::ImageLayout::eDepthStencilAttachmentOptimal:
                return vk::PipelineStageFlagBits::eLateFragmentTests;
            case vk::ImageLayout::ePreinitialized:
            return vk::PipelineStageFlagBits::eHost;
            default:
                throw std::runtime_error("[copyImage] No applicable pipeline stage for layout " +
                                         vk::to_string(layout));
            }
        }

        void recordSetLayout(CommandLayoutTransitionData& data,
                             const vk::CommandBuffer& command_buffer,
                             const vk::ImageLayout& old_layout,
                             const vk::ImageLayout& new_layout,
                             IImage& image) {

            vk::ImageSubresourceRange subresource_range = getSubresourceRange(image);

            data.image_memory_barrier.setOldLayout(old_layout)
                .setNewLayout(new_layout)
                .setImage(image.getImage())
                .setSubresourceRange(subresource_range)
                .setSrcAccessMask(getAccessMask(old_layout))
                .setDstAccessMask(getAccessMask(new_layout));

            command_buffer.pipelineBarrier(getPipelineStageFromLayout(old_layout),
                                           getPipelineStageFromLayout(new_layout),
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

        void recordImageToBufferCopy(const vk::CommandBuffer& command_buffer,
                                     const IImage& src,
                                     const IBuffer& dst) {
            vk::BufferImageCopy buffer_image_copy = createImageToBufferCopyInfo(src, dst);

            command_buffer
                .copyImageToBuffer(src.getImage(),
                                   vk::ImageLayout::eTransferSrcOptimal,
                                   dst.getBuffer(),
                                   1,
                                   &buffer_image_copy);
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

        recordSetLayout(auxillary_data.src_initial_transition_data,
                        command.buffer,
                        src.getIntendedLayout(),
                        vk::ImageLayout::eTransferSrcOptimal,
                        src);
        recordSetLayout(auxillary_data.dst_initial_transition_data,
                        command.buffer,
                        vk::ImageLayout::eUndefined,
                        vk::ImageLayout::eTransferDstOptimal,
                        dst);

        recordImageCopy(command.buffer,
                        src,
                        dst);

        recordSetLayout(auxillary_data.src_final_transition_data,
                        command.buffer,
                        vk::ImageLayout::eTransferSrcOptimal,
                        src.getIntendedLayout(),
                        src);

        recordSetLayout(auxillary_data.dst_final_transition_data,
                        command.buffer,
                        vk::ImageLayout::eTransferDstOptimal,
                        dst.getIntendedLayout(),
                        dst);

        command.buffer.end();
    }


    void recordCopyImageToBuffer(IImage& src,
                                 IBuffer& dst,
                                 CopyImageToBufferAuxillaryData& auxillary_data,
                                 const Command& command,
                                 const vk::Device& device,
                                 const vk::Queue& queue) {
        vk::CommandBufferBeginInfo begin_info;
        command.buffer.begin(begin_info);

        recordSetLayout(auxillary_data.src_initial_transition_data,
                        command.buffer,
                        src.getIntendedLayout(),
                        vk::ImageLayout::eTransferSrcOptimal,
                        src);
        recordImageToBufferCopy(command.buffer,
                                src,
                                dst);
        recordSetLayout(auxillary_data.src_final_transition_data,
                        command.buffer,
                        vk::ImageLayout::eTransferSrcOptimal,
                        src.getIntendedLayout(),
                        src);

        command.buffer.end();
    }
};
