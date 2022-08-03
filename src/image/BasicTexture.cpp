#include "./BasicTexture.hpp"

#include "./samplerUtil.hpp"
#include "../resource/ResourceWriteImageAuxillaryData.hpp"

#include <iostream>

namespace wg::internal {

    /**
     * BasicTextureSetup
     */

    BasicTextureSetup::BasicTextureSetup(const BasicImageSetup& image_settings)
        : image_settings(image_settings) { }

    /**
     * BasicTexture
     */

    BasicTexture::BasicTexture(const vk::Extent2D& dimensions,
                               const BasicTextureSetup& setup,
                               std::shared_ptr<const DeviceManager> device_manager)
        : BasicImage(dimensions,
                     setup.image_settings,
                     device_manager) {

        this->sampler = samplerUtil::createBasicSampler({}, device_manager->getDevice());
    }

    vk::Sampler BasicTexture::getSampler() const {
        return this->sampler;
    }

    std::unique_ptr<IResourceWriteAuxillaryData>
    BasicTexture::writeDescriptorUpdate(vk::WriteDescriptorSet& write_info) const {
        auto aux_data = std::make_unique<ResourceWriteImageAuxillaryData>();
        aux_data->image_write_info.sampler = this->sampler;
        aux_data->image_write_info.imageView = this->getView();
        aux_data->image_write_info.imageLayout = this->getIntendedLayout();

        write_info
            .setDescriptorCount(1)
            .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
            .setPImageInfo(&aux_data->image_write_info)
            .setDstArrayElement(0);

        return aux_data;
    }

    BasicTexture::~BasicTexture() {
        this->device_manager->getDevice().destroy(this->sampler);
    }
};
