#pragma once

#include <memory>

#include "./core/ChainReel.hpp"

#include "./framebuffer/IFramebuffer.hpp"
#include "./framebuffer/IFramebufferChain.hpp"

#include "./draw_pass/IDrawPass.hpp"
#include "./draw_pass/BasicDrawPassSettings.hpp"

#include "./image/ImageCopier.hpp"
#include "./image/ImageChainCopier.hpp"
#include "./image/ITexture.hpp"
#include "./image/ITextureChain.hpp"
#include "./image/BasicTexture.hpp"
#include "./image/InternallyStagedTexture.hpp"
#include "./image/StorageTexture.hpp"

#include "./resource/IUniform.hpp"
#include "./resource/IUniformChain.hpp"
#include "./resource/IResourceSetChain.hpp"

#include "./pipeline/IShader.hpp"
#include "./pipeline/IPipeline.hpp"
#include "./pipeline/ShaderStage.hpp"
#include "./pipeline/VertexAttribDesc.hpp"
#include "./pipeline/ComponentType.hpp"
#include "./pipeline/BasicPipelineSetup.hpp"
#include "./pipeline/IComputePipeline.hpp"

#include "./buffer/IVertexBuffer.hpp"
#include "./buffer/IIndexBuffer.hpp"
#include "./buffer/IStorageBuffer.hpp"

namespace wg {
    typedef std::unique_ptr<internal::IFramebuffer> Framebuffer;
    typedef std::shared_ptr<internal::IFramebufferChain> FramebufferChain;

    typedef std::shared_ptr<internal::IDrawPass> DrawPassPtr;
    typedef internal::BasicDrawPassSettings BasicDrawPassSettings;

    typedef std::shared_ptr<internal::ManagedSemaphoreChain> Semaphore;
    typedef internal::WaitSemaphoreSet WaitSemaphoreSet;
    typedef internal::SignalSemaphoreSet SignalSemaphoreSet;

    typedef std::shared_ptr<internal::EventChain> EventChainPtr;

    typedef std::unique_ptr<internal::ImageCopier> ImageCopierPtr;
    typedef std::shared_ptr<internal::ImageChainCopier> ImageChainCopierPtr;

    typedef std::shared_ptr<internal::IResourceSetChain> ResourceSetChainPtr;

    template<typename T>
    using UniformPtr = std::shared_ptr<internal::IUniform<T>>;

    template<typename T>
    using UniformChainPtr = std::shared_ptr<internal::IUniformChain<T>>;

    typedef std::shared_ptr<internal::ITexture> TexturePtr;
    typedef std::shared_ptr<internal::InternallyStagedTexture> SettableTexturePtr;
    typedef std::shared_ptr<internal::StorageTexture> StorageTexturePtr;
    typedef std::shared_ptr<internal::ITextureChain> TextureChainPtr;
    typedef std::shared_ptr<internal::FramebufferTextureChain> FramebufferTextureChainPtr;

    typedef std::shared_ptr<internal::IShader> ShaderPtr;
    typedef std::shared_ptr<internal::IPipeline> PipelinePtr;
    typedef std::shared_ptr<internal::IComputePipeline> ComputePipelinePtr;

    typedef internal::BasicTextureSetup BasicTextureSetup;
    typedef internal::BasicPipelineSetup BasicPipelineSetup;

    typedef internal::ShaderStage ShaderStage;

    typedef internal::VertexAttribDesc VertexAttribDesc;
    typedef internal::ComponentType ComponentType;

    template<typename T>
    using VertexBufferPtr = std::shared_ptr<internal::IVertexBuffer<T>>;

    using BufferPtr = std::shared_ptr<internal::IBuffer>;

    typedef std::shared_ptr<internal::IIndexBuffer> IndexBufferPtr;

    template<typename T>
    using StorageBufferPtr = std::shared_ptr<internal::IStorageBuffer<T>>;

    typedef std::shared_ptr<internal::ChainReel> ChainReelPtr;
};
