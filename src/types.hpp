#pragma once

#include <memory>

#include "./framebuffer/IFramebuffer.hpp"
#include "./framebuffer/IFramebufferChain.hpp"

#include "./draw_pass/IDrawPass.hpp"
#include "./draw_pass/BasicDrawPassSettings.hpp"

#include "./image/ImageCopier.hpp"
#include "./image/ITexture.hpp"
#include "./image/BasicTexture.hpp"

#include "./resource/IUniform.hpp"

namespace wg {
    typedef std::unique_ptr<internal::IFramebuffer> Framebuffer;
    typedef std::shared_ptr<internal::IFramebufferChain> FramebufferChain;

    typedef std::shared_ptr<internal::IDrawPass> DrawPassPtr;
    typedef internal::BasicDrawPassSettings BasicDrawPassSettings;

    typedef std::shared_ptr<internal::ManagedSemaphoreChain> Semaphore;

    typedef std::unique_ptr<internal::ImageCopier> ImageCopierPtr;

    template<typename T>
    using Uniform = std::shared_ptr<internal::IUniform<T>>;

    typedef std::shared_ptr<internal::ITexture> TexturePtr;
    typedef internal::BasicTextureSetup BasicTextureSetup;
}
