#pragma once

#include "../framebuffer/IFramebufferChain.hpp"

#include "../resource.hpp"
#include "../buffer.hpp"

#include <memory>

#include "../sync/ISynchronizedQueueOperation.hpp"

namespace wg::internal {
    class IDrawPass : public virtual ISynchronizedQueueOperation {
    public:

        virtual void startRecording(std::shared_ptr<IFramebufferChain> framebufferChain) = 0;
        virtual void recordDraw(const std::vector<const Buffer*>& buffers, const IndexBuffer* ind_buf,
                        const std::vector<ResourceSet*>& sets, uint32_t instanceCount = 1) = 0;
        virtual void endRecording() = 0;

        virtual void render() = 0;

        virtual ~IDrawPass();
    };
};
