#include "./deviceEvaluation.hpp"

#include "./queueUtils.hpp"

namespace wg::internal {

    namespace {
        uint32_t scoreIndices(const QueueIndices& queue_indices) {
            uint32_t score = 0;
            if (queue_indices.graphics != -1) {
                score += GRAPHICS_IMPORTANCE;
            }

            if (queue_indices.present != -1) {
                score += PRESENT_IMPORTANCE;
            }

            if (queue_indices.compute != -1) {
                score += COMPUTE_IMPORTANCE;
            }

            return score;
        }
    }

    uint32_t evaluatePhysicalDevice(vk::PhysicalDevice physical_device,
                                    vk::SurfaceKHR surface) {
        QueueIndices indices = getQueueIndicesForDevice(physical_device, surface);
        return scoreIndices(indices);
    }

    uint32_t evaluatePhysicalDeviceWithoutSurface(vk::PhysicalDevice physical_device) {
        QueueIndices indices = getQueueIndicesForDeviceWithoutSurface(physical_device);
        return scoreIndices(indices);
    }

};
