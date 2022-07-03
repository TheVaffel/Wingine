#pragma once

#include "./IFramebufferChain.hpp"
#include "./IFramebuffer.hpp"

#include "../QueueManager.hpp"

#include "../image/ITextureChain.hpp"
#include "../render_pass/CompatibleRenderPassRegistry.hpp"

#include "../sync/SignalAndWaitSemaphores.hpp"

#include "./FramebufferTexture.hpp"

namespace wg::internal {
    class FramebufferTextureChain : public IFramebufferChain, public ITextureChain {

        IndexCounter framebuffer_index;

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

        virtual ITexture& getTextureAt(uint32_t index) override;

        virtual uint32_t getNumFramebuffers() const override;
        virtual const IFramebuffer& getFramebuffer(uint32_t index) const override;

        virtual const IFramebuffer& getCurrentFramebuffer() const override;

        virtual void swapFramebuffer() override;

        virtual void swap() override;
        virtual uint32_t getCurrentIndex() const override;
        virtual uint32_t getNumResources() const override;

        virtual IResource& getResourceAt(uint32_t index) override;

        virtual void setPresentWaitSemaphores(WaitSemaphoreSet&& semaphores) override;
        virtual SemaphoreChainPtr addSignalImageAcquiredSemaphore() override;
        virtual void setSignalImageAcquiredSemaphores(SignalSemaphoreSet&& semaphores) override;

        virtual SignalAndWaitSemaphores& getSemaphores() override;
    };
};
