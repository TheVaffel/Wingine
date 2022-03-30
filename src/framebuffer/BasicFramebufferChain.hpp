#pragma once

#include "./IFramebufferChain.hpp"

#include "../DeviceManager.hpp"
#include "../render_pass/CompatibleRenderPassRegistry.hpp"

#include <concepts>

namespace wg::internal {

    template<typename T>
    concept CFramebuffer = std::derived_from<T, IFramebuffer>;

    template <CFramebuffer T>
    class BasicFramebufferChain : public IFramebufferChain {

        std::vector<std::unique_ptr<IFramebuffer>> framebuffers;
        uint32_t current_framebuffer;

    public:

        template<typename ...Args>
        BasicFramebufferChain(uint32_t count, Args&&... arguments);

        template<typename ...Args>
        static std::unique_ptr<BasicFramebufferChain> createFramebufferSet(uint32_t count, Args&&... arguments);

        virtual uint32_t getNumFramebuffers() const;
        virtual const IFramebuffer& getFramebuffer(uint32_t index) const;
        virtual IFramebuffer& getFramebuffer(uint32_t index);

        virtual const IFramebuffer& getCurrentFramebuffer() const;
        virtual IFramebuffer& getCurrentFramebuffer();

        virtual void swapFramebuffer();
    };
};


#include "./BasicFramebufferChain.impl.hpp"
