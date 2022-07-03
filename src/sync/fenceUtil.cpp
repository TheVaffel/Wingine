#include "./fenceUtil.hpp"

#include "../util/log.hpp"

#include <iostream>

namespace wg::internal::fenceUtil {

    void awaitFence(const vk::Fence& fence,
                    const vk::Device& device) {

        _wassert_result(device.waitForFences(1,
                                             &fence,
                                             true,
                                             (uint64_t)1e9),
                        "wait for fence");
    }

    void resetFence(const vk::Fence& fence,
                    const vk::Device& device) {

        _wassert_result(device.resetFences(1, &fence),
                        "reset fence");
    }

    void awaitAndResetFence(const vk::Fence& fence,
                            const vk::Device& device) {

        awaitFence(fence, device);
        resetFence(fence, device);
    }
};
