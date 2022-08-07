#include <vulkan/vulkan.hpp>

namespace wg::internal {

    const int GRAPHICS_IMPORTANCE = 4;
    const int PRESENT_IMPORTANCE = 2;
    const int COMPUTE_IMPORTANCE = 1;

    uint32_t evaluatePhysicalDevice(vk::PhysicalDevice physical_device,
                                    vk::SurfaceKHR surface);

    uint32_t evaluatePhysicalDeviceWithoutSurface(vk::PhysicalDevice physical_device);
};
