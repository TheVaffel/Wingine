#include "./FramebufferTexture.hpp"

#include "./BasicFramebuffer.hpp"
#include "../image/samplerUtil.hpp"

#include "../resource/ResourceWriteImageAuxillaryData.hpp"

namespace wg::internal {
    FramebufferTexture::FramebufferTexture(const vk::Extent2D& dimensions,
                                           bool depth_only,
                                           std::shared_ptr<const DeviceManager> device_manager,
                                           CompatibleRenderPassRegistry& render_pass_registry)
        : device_manager(device_manager) {
        BasicFramebufferSetup setup;
        setup.setDepthOnly(depth_only)
            .setSamplable(true);

        this->framebuffer = std::make_shared<BasicFramebuffer>(dimensions,
                                                               setup,
                                                               device_manager,
                                                               render_pass_registry);
        this->sampler = samplerUtil::createBasicSampler({}, device_manager->getDevice());
    }

    const IImage& FramebufferTexture::getRelevantFramebufferImage() const {
        return this->framebuffer->hasColorImage() ?
            this->framebuffer->getColorImage() :
            this->framebuffer->getDepthImage();
    }

    IImage& FramebufferTexture::getRelevantFramebufferImage() {
        return this->framebuffer->hasColorImage() ?
            this->framebuffer->getColorImage() :
            this->framebuffer->getDepthImage();
    }


    vk::Extent2D FramebufferTexture::getDimensions() const {
        return this->framebuffer->getDimensions();
    }

    bool FramebufferTexture::hasColorImage() const {
        return this->framebuffer->hasColorImage();
    }

    const IImage& FramebufferTexture::getColorImage() const {
        return this->framebuffer->getColorImage();
    }

    bool FramebufferTexture::hasDepthImage() const {
        return this->framebuffer->hasDepthImage();
    }

    const IImage& FramebufferTexture::getDepthImage() const {
        return this->framebuffer->getDepthImage();
    }

    const vk::Framebuffer& FramebufferTexture::getFramebuffer() const {
        return this->framebuffer->getFramebuffer();
    }

    vk::Sampler FramebufferTexture::getSampler() const {
        return this->sampler;
    }

    const vk::Image FramebufferTexture::getImage() const {
        return this->getRelevantFramebufferImage().getImage();
    }

    const vk::DeviceMemory FramebufferTexture::getMemory() const {
        return this->getRelevantFramebufferImage().getMemory();
    }

    const vk::ImageView FramebufferTexture::getView() const {
        return this->getRelevantFramebufferImage().getView();
    }

    const vk::ImageAspectFlagBits FramebufferTexture::getDefaultAspect() const {
        return this->getRelevantFramebufferImage().getDefaultAspect();
    }

    const vk::ImageLayout FramebufferTexture::getIntendedLayout() const {
        return this->getRelevantFramebufferImage().getIntendedLayout();
    }

    [[nodiscard]]
    std::unique_ptr<IResourceWriteAuxillaryData>
    FramebufferTexture::writeDescriptorUpdate(vk::WriteDescriptorSet& write_info) const {
        auto aux_data = std::make_unique<ResourceWriteImageAuxillaryData>();
        aux_data->image_write_info.sampler = this->sampler;
        aux_data->image_write_info.imageView = this->getView();
        aux_data->image_write_info.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

        write_info
            .setDescriptorCount(1)
            .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
            .setPImageInfo(&aux_data->image_write_info)
            .setDstArrayElement(0);

        return aux_data;
    }

    FramebufferTexture::~FramebufferTexture() {
        this->device_manager->getDevice().destroy(this->sampler);
    }
};
