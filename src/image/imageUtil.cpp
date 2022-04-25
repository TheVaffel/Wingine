#include <vulkan/vulkan.hpp>

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

    vk::Image createFramebufferColorImage(const vk::Extent2D& dimensions,
                                          const vk::Format& format,
                                          const vk::Device& device) {

        const ImageParameters parameters;
        return createImageRaw(device,
                              dimensions,
                              vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
                              format,
                              parameters);
    }

    vk::Image createFramebufferDepthImage(const vk::Extent2D& dimensions,
                                          const vk::Format& format,
                                          const vk::Device& device) {
        const ImageParameters parameters;
        return createImageRaw(device,
                              dimensions,
                              vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc,
                              format,
                              parameters);
    }


    vk::Image createHostAccessibleColorImage(const vk::Extent2D& dimensions,
                                             const vk::Format& format,
                                             const vk::Device& device) {
        ImageParameters parameters;
        parameters.image_tiling = vk::ImageTiling::eLinear;
        return createImageRaw(device,
                              dimensions,
                              vk::ImageUsageFlagBits::eTransferDst,
                              format,
                              parameters);
    }


    /*
     * Image views
     */

    vk::ImageView createColorImageView(const vk::Image& image,
                                       const vk::Format& format,
                                       const vk::Device& device) {
        vk::ImageViewCreateInfo ivci = getDefaultImageViewCreateInfo(image);
        ivci.setFormat(format)
            .setSubresourceRange({
                    vk::ImageAspectFlagBits::eColor,
                    0, // base mip level
                    1, // level count
                    0, // base array layer
                    1  // layer count
                });

        return device.createImageView(ivci);
    }

    vk::ImageView createDepthImageView(const vk::Image& image,
                                       const vk::Format& format,
                                       const vk::Device& device) {
        vk::ImageViewCreateInfo ivci = getDefaultImageViewCreateInfo(image);
        ivci.setFormat(format)
            .setSubresourceRange({
                    vk::ImageAspectFlagBits::eDepth,
                    0,
                    1,
                    0,
                    1
                });

        return device.createImageView(ivci);
    }
};
