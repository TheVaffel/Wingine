#pragma once

#include "./ITextureChain.hpp"

#include "./BasicTexture.hpp"

#include "../framebuffer/IFramebufferChain.hpp"

namespace wg::internal {
    class BasicTextureChain : public virtual ITextureChain {

        IndexCounter texture_index;
        std::vector<std::shared_ptr<ITexture>> textures;

        std::shared_ptr<const DeviceManager> device_manager;

    public:
        BasicTextureChain(uint32_t count,
                          const vk::Extent2D& dimensions,
                          const BasicTextureSetup& setup,
                          std::shared_ptr<const DeviceManager> device_manager);

        virtual void swap();
        virtual uint32_t getCurrentIndex() const;
        virtual uint32_t getNumResources() const;

        virtual ITexture& getTextureAt(uint32_t index);
        virtual IResource& getResourceAt(uint32_t index);
    };
};
