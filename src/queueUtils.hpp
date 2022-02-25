#include <vulkan/vulkan.hpp>

namespace wg::internal {

    /*
     * QueueIndices data struct holding some indices of relevant queue families
     */
    struct QueueIndices {
        int graphics = -1,
            present = -1,
            compute = -1;
    };

    /*
     * Returns the queue indices
     */

    QueueIndices getQueueIndicesForDevice(vk::PhysicalDevice physical_device,
                                          vk::SurfaceKHR surface);

    std::vector<vk::DeviceQueueCreateInfo> getDeviceQueueCreateInfos(const QueueIndices& indices);
};
