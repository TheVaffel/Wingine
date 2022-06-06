#pragma once

#include "./ITexture.hpp"
#include "./IImage.hpp"

#include "../resource/IResourceChain.hpp"
#include "../sync/ISynchronizedQueueOperation.hpp"

#include "../framebuffer/IFramebufferChain.hpp"

namespace wg::internal {
    class ITextureChain : public IResourceChain {
    public:
        virtual ITexture& getTextureAt(uint32_t index) = 0;

        virtual ~ITextureChain() = default;
    };
};
