#include "./BasicTextureChain.hpp"

#include "./BasicTexture.hpp"

#include <flawed_assert.hpp>

namespace wg::internal {
    BasicTextureChain::BasicTextureChain(uint32_t count,
                                         const vk::Extent2D& dimensions,
                                         const BasicTextureSetup& setup,
                                         std::shared_ptr<const DeviceManager> device_manager)
        : ElementChainBase(count),
          device_manager(device_manager) {

        this->textures.resize(count);

        for (uint32_t i = 0; i < count; i++) {
            this->textures[i] = std::make_unique<BasicTexture>(dimensions, setup, device_manager);
        }
    }

    IResource& BasicTextureChain::getResourceAt(uint32_t index) {
        return *this->textures[index];
    }

    ITexture& BasicTextureChain::getTextureAt(uint32_t index) {
        return *this->textures[index];
    }
};
