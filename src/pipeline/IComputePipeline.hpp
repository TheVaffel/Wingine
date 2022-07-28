#pragma once

#include "../sync/ISynchronizedQueueOperation.hpp"
#include "../resource/IResourceSetChain.hpp"

namespace wg::internal {
    class IComputePipeline : public virtual ISynchronizedQueueOperation {
    public:
        virtual void execute(const std::vector<std::shared_ptr<IResourceSetChain>>& resources,
                             uint32_t width,
                             uint32_t height = 1,
                             uint32_t depth = 1) = 0;

        virtual void awaitExecution() = 0;
    };
};
