#pragma once

#include "./SignalAndWaitSemaphores.hpp"

namespace wg::internal {
    class ISynchronizedQueueOperation {
    public:
        virtual SignalAndWaitSemaphores& getSemaphores() = 0;
    };
};
