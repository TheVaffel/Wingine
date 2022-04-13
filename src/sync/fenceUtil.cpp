#include "./fenceUtil.hpp"

#include "../log.hpp"

#include <chrono>
#include <iostream>

namespace wg::internal::fenceUtil {

    void awaitFence(const vk::Fence& fence,
                    const vk::Device& device) {


        std::chrono::high_resolution_clock clock;
        auto start = clock.now();
        _wassert_result(device.waitForFences(1,
                                             &fence,
                                             true,
                                             (uint64_t)1e9),
                        "wait for fence");

        auto end = clock.now();
        std::cout << "Waited for fence for " << std::chrono::duration<double>(end - start).count() * 1000.0 << " ms" << std::endl;
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
