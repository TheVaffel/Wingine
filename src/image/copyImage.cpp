#include "./copyImage.hpp"

#include "./imageUtil.hpp"

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

        imageUtil::recordSetLayout(auxillary_data.src_initial_transition_data,
                                   command.buffer,
                                   src.getIntendedLayout(),
                                   vk::ImageLayout::eTransferSrcOptimal,
                                   src);
        imageUtil::recordSetLayout(auxillary_data.dst_initial_transition_data,
                                   command.buffer,
                                   vk::ImageLayout::eUndefined,
                                   vk::ImageLayout::eTransferDstOptimal,
                                   dst);

        recordImageCopy(command.buffer,
                        src,
                        dst);

        imageUtil::recordSetLayout(auxillary_data.src_final_transition_data,
                                   command.buffer,
                                   vk::ImageLayout::eTransferSrcOptimal,
                                   src.getIntendedLayout(),
                                   src);

        imageUtil::recordSetLayout(auxillary_data.dst_final_transition_data,
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

        imageUtil::recordSetLayout(auxillary_data.src_initial_transition_data,
                                   command.buffer,
                                   src.getIntendedLayout(),
                                   vk::ImageLayout::eTransferSrcOptimal,
                                   src);
        recordImageToBufferCopy(command.buffer,
                                src,
                                dst);

        imageUtil::recordSetLayout(auxillary_data.src_final_transition_data,
                                   command.buffer,
                                   vk::ImageLayout::eTransferSrcOptimal,
                                   src.getIntendedLayout(),
                                   src);

        command.buffer.end();
    }
};
