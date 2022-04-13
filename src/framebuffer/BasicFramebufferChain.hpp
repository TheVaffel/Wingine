#pragma once

#include "./IFramebufferChain.hpp"

#include "../DeviceManager.hpp"
#include "../QueueManager.hpp"
#include "../render_pass/CompatibleRenderPassRegistry.hpp"

#include <concepts>

namespace wg::internal {

    template<typename T>
    concept CFramebuffer = std::derived_from<T, IFramebuffer>;

    template <CFramebuffer T>
    class BasicFramebufferChain : public IFramebufferChain {

        std::shared_ptr<const DeviceManager> device_manager;
        std::shared_ptr<const QueueManager> queue_manager;

        std::vector<std::unique_ptr<IFramebuffer>> framebuffers;
        uint32_t current_framebuffer;

        SemaphoreSet wait_semaphore_set;
        SemaphoreSet signal_semaphore_set;

    public:

        template<typename ...Args>
        BasicFramebufferChain(uint32_t count,
                              std::shared_ptr<const DeviceManager> device_manager,
                              std::shared_ptr<const QueueManager> queue_manager,
                              Args&&... arguments);

        template<typename ...Args>
        static std::unique_ptr<BasicFramebufferChain> createFramebufferSet(uint32_t count, Args&&... arguments);

        virtual uint32_t getNumFramebuffers() const;
        virtual const IFramebuffer& getFramebuffer(uint32_t index) const;
        virtual IFramebuffer& getFramebuffer(uint32_t index);

        virtual const IFramebuffer& getCurrentFramebuffer() const;
        virtual IFramebuffer& getCurrentFramebuffer();

        virtual void swapFramebuffer();

        virtual void setPresentWaitSemaphores(const SemaphoreSet& semaphores);
        virtual std::shared_ptr<ManagedSemaphoreChain> addSignalImageAcquiredSemaphore();
        virtual void setSignalImageAcquiredSemaphores(const SemaphoreSet& semaphores);
    };
};


#include "./BasicFramebufferChain.impl.hpp"
