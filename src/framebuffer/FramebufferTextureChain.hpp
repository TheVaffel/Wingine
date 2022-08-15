#pragma once

#include "./IFramebufferChain.hpp"
#include "./IFramebuffer.hpp"

#include "../core/QueueManager.hpp"
#include "../core/ElementChainBase.hpp"

#include "../image/ITextureChain.hpp"
#include "../render_pass/CompatibleRenderPassRegistry.hpp"

#include "../sync/SignalAndWaitSemaphores.hpp"

#include "./FramebufferTexture.hpp"

namespace wg::internal {
    class FramebufferTextureChain : public IFramebufferChain, public ITextureChain, public ElementChainBase {

        SignalAndWaitSemaphores semaphores;

        std::vector<std::shared_ptr<FramebufferTexture>> framebuffer_textures;

        std::shared_ptr<const QueueManager> queue_manager;

    public:

        FramebufferTextureChain(uint32_t count,
                                const vk::Extent2D& dimensions,
                                bool depth_only,
                                std::shared_ptr<const DeviceManager> device_manager,
                                std::shared_ptr<const QueueManager> queue_manager,
                                CompatibleRenderPassRegistry& render_pass_registry);

        virtual ITexture& getTextureAt(uint32_t index) final;

        virtual void swapToNextElement() final;

        virtual const IFramebuffer& getFramebuffer(uint32_t index) const final;
        virtual const IFramebuffer& getCurrentFramebuffer() const final;

        virtual IResource& getResourceAt(uint32_t index) final;

        virtual void setPresentWaitSemaphores(WaitSemaphoreSet&& semaphores) final;
        virtual SemaphoreChainPtr addSignalImageAcquiredSemaphore() final;
        virtual void setSignalImageAcquiredSemaphores(SignalSemaphoreSet&& semaphores) final;

        virtual SignalAndWaitSemaphores& getSemaphores() final;
    };
};
