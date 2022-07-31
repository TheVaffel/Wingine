#include <vulkan/vulkan.hpp>

#include "./imageUtil.hpp"

#include "../sync/fenceUtil.hpp"
#include "../util/log.hpp"

#include <iostream>

namespace wg::internal::imageUtil {
    namespace {

        /*
         * Images
         */

        // Custom struct to better control default values for various parameters
        struct ImageParameters {
            vk::ImageTiling image_tiling = vk::ImageTiling::eOptimal;
            vk::ImageLayout initial_image_layout = vk::ImageLayout::eUndefined;
            vk::SharingMode sharing_mode = vk::SharingMode::eExclusive;
            vk::SampleCountFlagBits image_samples = vk::SampleCountFlagBits::e1;
            uint32_t mip_levels = 1;
            uint32_t array_layers = 1;
        };

        vk::Image createImageRaw(const vk::Device& device,
                                 const vk::Extent2D& dimensions,
                                 const vk::ImageUsageFlags& image_usage_flags,
                                 const vk::Format& image_format,
                                 const ImageParameters& parameters) {

            vk::ImageCreateInfo image_create_info;
            image_create_info
                .setExtent({ dimensions.width, dimensions.height, 1 })
                .setImageType(vk::ImageType::e2D)
                .setTiling(parameters.image_tiling)
                .setInitialLayout(parameters.initial_image_layout)
                .setUsage(image_usage_flags)
                .setFormat(image_format)
                .setSharingMode(parameters.sharing_mode)
                .setSamples(parameters.image_samples)
                .setMipLevels(parameters.mip_levels)
                .setArrayLayers(parameters.array_layers);

            return device.createImage(image_create_info);
        }

        /*
         * Image view
         */

        vk::ImageViewCreateInfo getDefaultImageViewCreateInfo(const vk::Image& image) {
            vk::ImageViewCreateInfo ivci;
            ivci.setImage(image)
                .setViewType(vk::ImageViewType::e2D)
                .setComponents({
                        vk::ComponentSwizzle::eR,
                        vk::ComponentSwizzle::eG,
                        vk::ComponentSwizzle::eB,
                        vk::ComponentSwizzle::eA
                    });
            return ivci;
        }

    };

    /*
     * Images
     */

    vk::Image createSimpleImage(const vk::Extent2D& dimensions,
                                const vk::ImageUsageFlags& usage,
                                const vk::Format& format,
                                const vk::ImageTiling& tiling,
                                const vk::Device& device) {
        ImageParameters parameters;
        parameters.image_tiling = tiling;
        return createImageRaw(device,
                              dimensions,
                              usage,
                              format,
                              parameters);
    }


    /*
     * Image views
     */

    vk::ImageView createImageView(const vk::Image& image,
                                  const vk::Format& format,
                                  const vk::ImageAspectFlagBits& aspect,
                                  const vk::Device& device) {
        vk::ImageViewCreateInfo ivci = getDefaultImageViewCreateInfo(image);
        ivci.setFormat(format)
            .setSubresourceRange({
                    aspect,
                    0, // base mip level
                    1, // level count
                    0, // base array layer
                    1  // layer count
                });

        return device.createImageView(ivci);
    }

    vk::ImageView createColorImageView(const vk::Image& image,
                                       const vk::Format& format,
                                       const vk::Device& device) {
        return createImageView(image, format, vk::ImageAspectFlagBits::eColor, device);
    }

    vk::ImageView createDepthImageView(const vk::Image& image,
                                       const vk::Format& format,
                                       const vk::Device& device) {

        return createImageView(image, format, vk::ImageAspectFlagBits::eDepth, device);
    }

    /*
     * Image layout
     */

    void initializeLayout(const IImage& image,
                          const vk::ImageLayout& layout,
                          const Command& command,
                          const vk::Queue& queue,
                          const vk::Device& device) {
        CommandLayoutTransitionData data;

        command.buffer.begin(vk::CommandBufferBeginInfo());
        imageUtil::recordSetLayout(data,
                                   command.buffer,
                                   vk::ImageLayout::eUndefined,
                                   layout,
                                   image);

        command.buffer.end();

        std::vector<vk::CommandBuffer> command_buffers = std::vector<vk::CommandBuffer> { command.buffer };

        vk::SubmitInfo si;
        si.setCommandBuffers(command_buffers);

        fenceUtil::awaitAndResetFence(command.fence, device);

        _wassert_result(queue.submit(1, &si, command.fence),
                        "submitting set-layout buffer");

        fenceUtil::awaitFence(command.fence, device);
    }


    /*
     * Recording
     */

    namespace {
        vk::ImageSubresourceRange getSubresourceRange(const IImage& image) {

            vk::ImageSubresourceRange subresource_range;
            subresource_range.setAspectMask(image.getDefaultAspect())
                .setLevelCount(1)
                .setBaseArrayLayer(0)
                .setLayerCount(1);
            return subresource_range;
        }


        vk::AccessFlagBits getAccessMask(const vk::ImageLayout& layout) {
            switch(layout) {
            case vk::ImageLayout::eColorAttachmentOptimal:
            case vk::ImageLayout::ePresentSrcKHR:
            case vk::ImageLayout::eGeneral:
                return vk::AccessFlagBits::eMemoryRead;
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
            case vk::ImageLayout::eUndefined:
                return vk::AccessFlagBits::eNone;
            default:
                throw std::runtime_error("[copyImage] No applicable access mask for layout " +
                                         vk::to_string(layout));
            }
        }

        vk::PipelineStageFlags getPipelineStageFromLayout(const vk::ImageLayout& layout) {
            switch (layout) {
            case vk::ImageLayout::eTransferDstOptimal:
            case vk::ImageLayout::eTransferSrcOptimal:
            case vk::ImageLayout::eGeneral:
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
            case vk::ImageLayout::eUndefined:
                return vk::PipelineStageFlagBits::eTopOfPipe;
            default:
                throw std::runtime_error("[copyImage] No applicable pipeline stage for layout " +
                                         vk::to_string(layout));
            }
        }
    };

    void recordSetLayout(CommandLayoutTransitionData& data,
                         const vk::CommandBuffer& command_buffer,
                         const vk::ImageLayout& old_layout,
                         const vk::ImageLayout& new_layout,
                         const IImage& image) {

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
};
