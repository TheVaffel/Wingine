#pragma once

#include <memory>

#include "./framebuffer/IFramebuffer.hpp"
#include "./framebuffer/IFramebufferChain.hpp"

#include "./draw_pass/IDrawPass.hpp"
#include "./draw_pass/BasicDrawPassSettings.hpp"

#include "./image/ImageCopier.hpp"
#include "./image/ImageChainCopier.hpp"
#include "./image/ITexture.hpp"
#include "./image/ITextureChain.hpp"
#include "./image/BasicTexture.hpp"

#include "./resource/IUniform.hpp"
#include "./resource/IUniformChain.hpp"

#include "./resource/IResourceSetChain.hpp"

namespace wg {
    typedef std::unique_ptr<internal::IFramebuffer> Framebuffer;
    typedef std::shared_ptr<internal::IFramebufferChain> FramebufferChain;

    typedef std::shared_ptr<internal::IDrawPass> DrawPassPtr;
    typedef internal::BasicDrawPassSettings BasicDrawPassSettings;

    typedef std::shared_ptr<internal::ManagedSemaphoreChain> Semaphore;

    typedef std::unique_ptr<internal::ImageCopier> ImageCopierPtr;
    typedef std::shared_ptr<internal::ImageChainCopier> ImageChainCopierPtr;

    typedef std::shared_ptr<internal::IResourceSetChain> ResourceSetChainPtr;

    template<typename T>
    using UniformPtr = std::shared_ptr<internal::IUniform<T>>;

    template<typename T>
    using UniformChainPtr = std::shared_ptr<internal::IUniformChain<T>>;

    typedef std::shared_ptr<internal::ITexture> TexturePtr;
    typedef std::shared_ptr<internal::ITextureChain> TextureChainPtr;
    typedef internal::BasicTextureSetup BasicTextureSetup;
}
