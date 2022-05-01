#include "./DrawPassBase.hpp"

namespace wg::internal {
    DrawPassBase::DrawPassBase(uint32_t num_semaphores,
                               std::shared_ptr<const DeviceManager> device_manager)
        : SynchronizedQueueOperationBase(num_semaphores, device_manager),
          device_manager(device_manager) { }
};
