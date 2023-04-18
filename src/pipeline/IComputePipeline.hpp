#pragma once

#include "../sync/ISynchronizedQueueOperation.hpp"
#include "../resource/ResourceBinding.hpp"

namespace wg::internal {
    class IComputePipeline : public virtual ISynchronizedQueueOperation {
    public:
        virtual void execute(const std::vector<std::vector<ResourceBinding>>& resources,
                             uint32_t width,
                             uint32_t height = 1,
                             uint32_t depth = 1) = 0;

        virtual void awaitExecution() = 0;
    };
};
