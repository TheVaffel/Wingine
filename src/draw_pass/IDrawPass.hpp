#pragma once

#include "../command/CommandChainController.hpp"

#include <memory>

#include "../sync/ISynchronizedQueueOperation.hpp"

namespace wg::internal {
    class IDrawPass : public virtual ISynchronizedQueueOperation {
    public:

        virtual CommandChainController& getCommandChain() = 0;

        virtual void render() = 0;

        virtual ~IDrawPass() = default;
    };
};
