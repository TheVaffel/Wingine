#pragma once

#include "./FramebufferChainBase.hpp"

#include "../core/DeviceManager.hpp"
#include "../core/QueueManager.hpp"
#include "../render_pass/CompatibleRenderPassRegistry.hpp"

#include <concepts>

namespace wg::internal {

    template<typename T>
    concept CFramebuffer = std::derived_from<T, IFramebuffer>;

    template <CFramebuffer T>
    class BasicFramebufferChain : public FramebufferChainBase {

        std::vector<std::unique_ptr<IFramebuffer>> framebuffers;
        IndexCounter framebuffer_index;

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
        virtual const IFramebuffer& getCurrentFramebuffer() const;

        virtual void swapFramebuffer();
    };
};


#include "./BasicFramebufferChain.impl.hpp"
