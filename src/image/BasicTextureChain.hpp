#pragma once

#include "./ITextureChain.hpp"

#include "./BasicTexture.hpp"

#include "../framebuffer/IFramebufferChain.hpp"
#include "../core/ElementChainBase.hpp"

namespace wg::internal {
    class BasicTextureChain : public virtual ITextureChain, public ElementChainBase {

        std::vector<std::shared_ptr<ITexture>> textures;

        std::shared_ptr<const DeviceManager> device_manager;

    public:
        BasicTextureChain(uint32_t count,
                          const vk::Extent2D& dimensions,
                          const BasicTextureSetup& setup,
                          std::shared_ptr<const DeviceManager> device_manager);

        virtual ITexture& getTextureAt(uint32_t index) override;
        virtual IResource& getResourceAt(uint32_t index) override;
    };
};
