#pragma once

#include "./ITextureChain.hpp"

namespace wg::internal {
    class BasicStreamingTextureChain : public IStreamingTextureChain {
        std::shared_ptr<DeviceManager> device_manager;

    public:
        BasicStreamingTextureChain(uint32_t count,
                                   const vk::Extent2D& dimensions,
                                   std::shared_ptr<DeviceManager> device_manager);

        virtual std::shared_ptr<ITexture> getCurrentTexture();

        virtual ITexture& getTexture(uint32_t index);
        virtual uint32_t getNumTextures() const;
    };
};
