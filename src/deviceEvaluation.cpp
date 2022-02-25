#include "./deviceEvaluation.hpp"

#include "./queueUtils.hpp"

namespace wg::internal {

    uint32_t evaluatePhysicalDevice(vk::PhysicalDevice physical_device,
                                    vk::SurfaceKHR surface) {
        uint32_t score = 0;

        QueueIndices indices = getQueueIndicesForDevice(physical_device, surface);
        if (indices.graphics != -1) {
            score += GRAPHICS_IMPORTANCE;
        }

        if (indices.present != -1) {
            score += PRESENT_IMPORTANCE;
        }

        if (indices.compute != -1) {
            score += COMPUTE_IMPORTANCE;
        }

        return score;
    }
};
