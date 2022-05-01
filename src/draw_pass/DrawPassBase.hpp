#pragma once

#include "./IDrawPass.hpp"
#include "../sync/SynchronizedQueueOperationBase.hpp"

namespace wg::internal {
    class DrawPassBase : public IDrawPass, public SynchronizedQueueOperationBase {
    protected:
        std::shared_ptr<const DeviceManager> device_manager;

        DrawPassBase(uint32_t num_semaphores,
                     std::shared_ptr<const DeviceManager> device_manager);
    };
};
