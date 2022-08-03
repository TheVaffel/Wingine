#pragma once

#include "./ITexture.hpp"
#include "./BasicImage.hpp"

#include "../DeviceManager.hpp"
#include "./ImageCopier.hpp"

namespace wg::internal {

    struct BasicTextureSetup {
        BasicImageSetup image_settings;

        BasicTextureSetup(const BasicImageSetup& image_settings);
    };

    class BasicTexture : public virtual ITexture, public BasicImage {

        vk::Sampler sampler;
    public:

        BasicTexture(const vk::Extent2D& dimensions,
                     const BasicTextureSetup& setup,
                     std::shared_ptr<const DeviceManager> device_manager);

        virtual vk::Sampler getSampler() const override;
        virtual std::unique_ptr<IResourceWriteAuxillaryData> writeDescriptorUpdate(vk::WriteDescriptorSet& write_info) const override;

        ~BasicTexture();
    };
};
